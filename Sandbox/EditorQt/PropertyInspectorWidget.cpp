#include "PropertyInspectorWidget.hpp"
#include <Dan/Common/Math/MathTypes.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/PhysicsComponents.hpp>
#include <Dan/Common/Components/TagComponent.hpp>

#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <glm/gtc/quaternion.hpp>

using namespace entt::literals;

namespace Dan {

    PropertyInspectorWidget::PropertyInspectorWidget(entt::registry& registry, QWidget* parent)
        : QDockWidget("Property Inspector", parent), m_registry(registry)
    {
        setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

        m_containerWidget = new QWidget(this);
        m_mainLayout = new QVBoxLayout(m_containerWidget);
        m_mainLayout->setAlignment(Qt::AlignTop);

        setWidget(m_containerWidget);
    }

    void PropertyInspectorWidget::InspectEntity(entt::entity entity)
    {
        m_selectedEntity = entity;
        ClearLayout();

        if (m_selectedEntity == entt::null || !m_registry.valid(m_selectedEntity)) {
            m_mainLayout->addWidget(new QLabel("Nenhuma entidade selecionada.", m_containerWidget));
            return;
        }

        // 1. Cabeçalho Fixo: General (ID, Name, Layer)
        DrawGeneralHeader();

        // 2. Varre TODOS os componentes registrados dinamicamente via entt::meta
        for (auto [typeId, metaType] : entt::resolve())
        {
            // Ignora o TagComponent no loop dinâmico pois ele já é desenhado no General
            if (metaType.id() == "TagComponent"_hs) {
                continue;
            }

            if (auto getFunc = metaType.func("get"_hs); getFunc)
            {
                entt::meta_any compInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);

                if (compInstance) {
                    DrawMetaTypeComponent(metaType, compInstance);
                }
            }
        }

        // 3. Fallback Hardcoded para componentes dinâmicos (MeshComponent / TerrainComponent)
        if (auto* meshComp = m_registry.try_get<MeshComponent>(m_selectedEntity))
        {
            QGroupBox* group = new QGroupBox("Mesh Component", m_containerWidget);
            QFormLayout* formLayout = new QFormLayout(group);

            QString meshPath = meshComp->meshAsset ? QString::fromStdString(meshComp->meshAsset->GetVirtualPath()) : "Nenhum";
            formLayout->addRow("Malha:", new QLabel(meshPath));

            if (meshComp->material) {
                QString vsPath = meshComp->material->GetVertexShader() ? QString::fromStdString(meshComp->material->GetVertexShader()->GetVirtualPath()) : "Nenhum";
                QString fsPath = meshComp->material->GetFragmentShader() ? QString::fromStdString(meshComp->material->GetFragmentShader()->GetVirtualPath()) : "Nenhum";
                formLayout->addRow("Vertex Shader:", new QLabel(vsPath));
                formLayout->addRow("Fragment Shader:", new QLabel(fsPath));
            }

            m_mainLayout->addWidget(group);
        }
    }

    void PropertyInspectorWidget::DrawGeneralHeader()
    {
        QGroupBox* generalGroup = new QGroupBox("General", m_containerWidget);
        QFormLayout* formLayout = new QFormLayout(generalGroup);

        // Campo 1: ID da Entidade (Somente Leitura)
        uint32_t rawID = static_cast<uint32_t>(m_selectedEntity);
        QLabel* idLabel = new QLabel(QString::number(rawID), generalGroup);
        idLabel->setStyleSheet("color: #888888; font-weight: bold;");
        formLayout->addRow("ID:", idLabel);

        // Campo 2: Name (TagComponent)
        auto* tagComp = m_registry.try_get<TagComponent>(m_selectedEntity);
        QString currentName = (tagComp && !tagComp->tag.empty()) ? QString::fromStdString(tagComp->tag) : QString("Entity_%1").arg(rawID);

        QLineEdit* nameEdit = new QLineEdit(currentName, generalGroup);
        connect(nameEdit, &QLineEdit::editingFinished, [this, nameEdit]() {
            if (m_selectedEntity == entt::null || !m_registry.valid(m_selectedEntity)) return;

            std::string newName = nameEdit->text().toStdString();
            if (auto* tag = m_registry.try_get<TagComponent>(m_selectedEntity)) {
                tag->tag = newName;
            }
            else {
                m_registry.emplace<TagComponent>(m_selectedEntity, TagComponent{ newName });
            }

            // Notifica os outros painéis (SceneOutliner) para atualizar o rótulo
            emit EntityRenamed(m_selectedEntity, QString::fromStdString(newName));
            });
        formLayout->addRow("Name:", nameEdit);

        // Campo 3: Layer
        QComboBox* layerCombo = new QComboBox(generalGroup);
        layerCombo->addItems({ "Main Layer", "Environment", "Characters", "Props" });

        if (tagComp) {
            int index = layerCombo->findText(QString::fromStdString(tagComp->layer));
            if (index != -1) layerCombo->setCurrentIndex(index);
        }

        connect(layerCombo, &QComboBox::currentTextChanged, [this](const QString& text) {
            if (m_selectedEntity == entt::null || !m_registry.valid(m_selectedEntity)) return;

            if (auto* tag = m_registry.try_get<TagComponent>(m_selectedEntity)) {
                tag->layer = text.toStdString();
            }
            });
        formLayout->addRow("Layer:", layerCombo);

        m_mainLayout->addWidget(generalGroup);
    }

    void PropertyInspectorWidget::DrawMetaTypeComponent(entt::meta_type metaType, entt::meta_any compInstance)
    {
        std::string typeName(metaType.name());
        if (typeName.empty()) typeName = "Component";

        QGroupBox* group = new QGroupBox(QString::fromStdString(typeName), m_containerWidget);
        QFormLayout* formLayout = new QFormLayout(group);

        for (auto [fieldId, metaData] : metaType.data())
        {
            std::string fieldName(metaData.name());
            if (fieldName.empty()) continue;

            DrawFieldControl(formLayout, fieldName, metaData, compInstance);
        }

        m_mainLayout->addWidget(group);
    }

    void PropertyInspectorWidget::DrawFieldControl(QFormLayout* formLayout, const std::string& fieldName, entt::meta_data metaData, entt::meta_any compInstance)
    {
        entt::meta_any propValue = metaData.get(compInstance);
        if (!propValue) return;

        entt::meta_type propType = propValue.type();
        entt::meta_type parentType = compInstance.type();
        QString labelText = QString::fromStdString(fieldName);

        // 1. Campo float
        if (propType == entt::resolve<float>())
        {
            QDoubleSpinBox* spin = new QDoubleSpinBox();
            spin->setRange(-10000.0, 10000.0);
            spin->setSingleStep(0.1);
            spin->setValue(static_cast<double>(propValue.cast<float>()));

            QObject::connect(spin, &QDoubleSpinBox::valueChanged, [this, metaData, parentType](double val) mutable {
                if (auto getFunc = parentType.func("get"_hs); getFunc) {
                    entt::meta_any freshInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                    if (freshInstance) {
                        metaData.set(freshInstance, static_cast<float>(val));
                        if (auto assignFunc = parentType.func("assign"_hs); assignFunc) {
                            assignFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity, freshInstance);
                        }
                    }
                }
                });

            formLayout->addRow(labelText, spin);
        }
        // 2. Campo bool
        else if (propType == entt::resolve<bool>())
        {
            QCheckBox* checkBox = new QCheckBox();
            checkBox->setChecked(propValue.cast<bool>());

            QObject::connect(checkBox, &QCheckBox::toggled, [this, metaData, parentType](bool checked) mutable {
                if (auto getFunc = parentType.func("get"_hs); getFunc) {
                    entt::meta_any freshInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                    if (freshInstance) {
                        metaData.set(freshInstance, checked);
                        if (auto assignFunc = parentType.func("assign"_hs); assignFunc) {
                            assignFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity, freshInstance);
                        }
                    }
                }
                });

            formLayout->addRow(labelText, checkBox);
        }
        // 3. Campo Math::Vec3
        else if (propType == entt::resolve<Math::Vec3>())
        {
            Math::Vec3 vec = propValue.cast<Math::Vec3>();

            QWidget* vecWidget = new QWidget();
            QHBoxLayout* vecLayout = new QHBoxLayout(vecWidget);
            vecLayout->setContentsMargins(0, 0, 0, 0);

            auto createAxisSpin = [this, &vecLayout, metaData, parentType](float initialVal, auto axisSetter) mutable {
                QDoubleSpinBox* spin = new QDoubleSpinBox();
                spin->setRange(-10000.0, 10000.0);
                spin->setSingleStep(0.1);
                spin->setValue(static_cast<double>(initialVal));

                QObject::connect(spin, &QDoubleSpinBox::valueChanged, [this, metaData, parentType, axisSetter](double val) mutable {
                    if (auto getFunc = parentType.func("get"_hs); getFunc) {
                        entt::meta_any freshInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                        if (freshInstance) {
                            Math::Vec3 currentVec = metaData.get(freshInstance).cast<Math::Vec3>();
                            axisSetter(currentVec, static_cast<float>(val));
                            metaData.set(freshInstance, currentVec);

                            if (auto assignFunc = parentType.func("assign"_hs); assignFunc) {
                                assignFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity, freshInstance);
                            }
                        }
                    }
                    });

                vecLayout->addWidget(spin);
                };

            createAxisSpin(vec.x, [](Math::Vec3& v, float val) { v.x = val; });
            createAxisSpin(vec.y, [](Math::Vec3& v, float val) { v.y = val; });
            createAxisSpin(vec.z, [](Math::Vec3& v, float val) { v.z = val; });

            formLayout->addRow(labelText, vecWidget);
        }
        // 4. Campo glm::quat (Convertido para Euler)
        else if (propType == entt::resolve<glm::quat>())
        {
            glm::quat q = propValue.cast<glm::quat>();
            Math::Vec3 eulerDeg = glm::degrees(glm::eulerAngles(q));

            QWidget* rotWidget = new QWidget();
            QHBoxLayout* rotLayout = new QHBoxLayout(rotWidget);
            rotLayout->setContentsMargins(0, 0, 0, 0);

            auto createRotSpin = [this, &rotLayout, metaData, parentType](float initialVal, int axisIndex) mutable {
                QDoubleSpinBox* spin = new QDoubleSpinBox();
                spin->setRange(-360.0, 360.0);
                spin->setSingleStep(1.0);
                spin->setValue(static_cast<double>(initialVal));

                QObject::connect(spin, &QDoubleSpinBox::valueChanged, [this, metaData, parentType, axisIndex](double val) mutable {
                    if (auto getFunc = parentType.func("get"_hs); getFunc) {
                        entt::meta_any freshInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                        if (freshInstance) {
                            glm::quat currentQ = metaData.get(freshInstance).cast<glm::quat>();
                            Math::Vec3 currentEuler = glm::degrees(glm::eulerAngles(currentQ));

                            if (axisIndex == 0) currentEuler.x = static_cast<float>(val);
                            if (axisIndex == 1) currentEuler.y = static_cast<float>(val);
                            if (axisIndex == 2) currentEuler.z = static_cast<float>(val);

                            glm::quat newQ = glm::quat(glm::radians(currentEuler));
                            metaData.set(freshInstance, newQ);

                            if (auto assignFunc = parentType.func("assign"_hs); assignFunc) {
                                assignFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity, freshInstance);
                            }
                        }
                    }
                    });

                rotLayout->addWidget(spin);
                };

            createRotSpin(eulerDeg.x, 0);
            createRotSpin(eulerDeg.y, 1);
            createRotSpin(eulerDeg.z, 2);

            formLayout->addRow(labelText, rotWidget);
        }
        else if (propType == entt::resolve<MotionType>())
        {
            QComboBox* combo = new QComboBox();
            combo->addItem("Static", static_cast<int>(MotionType::Static));
            combo->addItem("Kinematic", static_cast<int>(MotionType::Kinematic));
            combo->addItem("Dynamic", static_cast<int>(MotionType::Dynamic));

            MotionType currentMt = propValue.cast<MotionType>();
            combo->setCurrentIndex(combo->findData(static_cast<int>(currentMt)));

            QObject::connect(combo, &QComboBox::currentIndexChanged, [this, metaData, parentType, combo](int index) mutable {
                if (auto getFunc = parentType.func("get"_hs); getFunc) {
                    entt::meta_any freshInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                    if (freshInstance) {
                        MotionType newMt = static_cast<MotionType>(combo->itemData(index).toInt());
                        metaData.set(freshInstance, newMt);

                        if (auto assignFunc = parentType.func("assign"_hs); assignFunc) {
                            assignFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity, freshInstance);
                        }
                    }
                }
                });

            formLayout->addRow(labelText, combo);
        }
    }

    void PropertyInspectorWidget::ClearLayout()
    {
        QLayoutItem* item;
        while ((item = m_mainLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
    }

} // namespace Dan