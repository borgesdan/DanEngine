#include "PropertyInspectorWidget.hpp"
#include <Dan/Common/Math/MathTypes.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/PhysicsComponents.hpp>
#include <Dan/Common/Components/TagComponent.hpp>

#include <QHBoxLayout>
#include <QToolButton>
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

        // 1. Cabeçalho Fixo "General"
        DrawGeneralHeader();

        // 2. Varre dinamicamente todos os componentes registrados via entt::meta
        for (auto [typeId, metaType] : entt::resolve())
        {
            if (metaType.id() == "TagComponent"_hs) {
                continue; // Exibido no cabeçalho General
            }

            if (auto getFunc = metaType.func("get"_hs); getFunc)
            {
                entt::meta_any compInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);

                if (compInstance) {
                    DrawMetaTypeComponent(metaType, compInstance);
                }
            }
        }

        // 3. Fallback para componentes complexos (MeshComponent)
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

        // 4. Desenha o Botão "+ Add Component" no rodapé
        DrawAddComponentButton();
    }

    void PropertyInspectorWidget::DrawGeneralHeader()
    {
        QGroupBox* generalGroup = new QGroupBox("General", m_containerWidget);
        QFormLayout* formLayout = new QFormLayout(generalGroup);

        uint32_t rawID = static_cast<uint32_t>(m_selectedEntity);
        QLabel* idLabel = new QLabel(QString::number(rawID), generalGroup);
        idLabel->setStyleSheet("color: #888888; font-weight: bold;");
        formLayout->addRow("ID:", idLabel);

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

            emit EntityRenamed(m_selectedEntity, QString::fromStdString(newName));
            });
        formLayout->addRow("Name:", nameEdit);

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

        QGroupBox* group = new QGroupBox(m_containerWidget);
        QVBoxLayout* groupLayout = new QVBoxLayout(group);
        groupLayout->setContentsMargins(6, 6, 6, 6);

        // Barra de Título Customizada com Botão de Remoção "X"
        QWidget* titleHeader = new QWidget(group);
        QHBoxLayout* titleLayout = new QHBoxLayout(titleHeader);
        titleLayout->setContentsMargins(0, 0, 0, 0);

        QLabel* titleLabel = new QLabel(QString::fromStdString(typeName), titleHeader);
        titleLabel->setStyleSheet("font-weight: bold; font-size: 11px;");
        titleLayout->addWidget(titleLabel);
        titleLayout->addStretch();

        // Não permite remover o Transform obrigatório das entidades
        if (metaType.id() != "Transform"_hs)
        {
            QToolButton* removeBtn = new QToolButton(titleHeader);
            removeBtn->setText("X");
            removeBtn->setStyleSheet("QToolButton { color: #ff5555; font-weight: bold; border: none; } QToolButton:hover { color: #ff0000; }");
            removeBtn->setToolTip("Remover Componente");

            connect(removeBtn, &QToolButton::clicked, [this, metaType]() {
                if (m_selectedEntity == entt::null || !m_registry.valid(m_selectedEntity)) return;

                if (auto removeFunc = metaType.func("remove"_hs); removeFunc) {
                    removeFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                    InspectEntity(m_selectedEntity); // Redesenha a UI
                }
                });

            titleLayout->addWidget(removeBtn);
        }

        groupLayout->addWidget(titleHeader);

        // Formulário de Campos
        QFormLayout* formLayout = new QFormLayout();
        groupLayout->addLayout(formLayout);

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

        auto updateFieldInECS = [this, parentType, metaData](auto newValue) {
            if (auto getFunc = parentType.func("get"_hs); getFunc) {
                entt::meta_any freshInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                if (freshInstance) {
                    metaData.set(freshInstance, newValue);
                    if (auto assignFunc = parentType.func("assign"_hs); assignFunc) {
                        assignFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity, freshInstance);
                    }
                }
            }
            };

        // 1. float
        if (propType == entt::resolve<float>())
        {
            QDoubleSpinBox* spin = new QDoubleSpinBox();
            spin->setRange(-10000.0, 10000.0);
            spin->setSingleStep(0.1);
            spin->setValue(static_cast<double>(propValue.cast<float>()));

            connect(spin, &QDoubleSpinBox::valueChanged, [updateFieldInECS](double val) {
                updateFieldInECS(static_cast<float>(val));
                });

            formLayout->addRow(labelText, spin);
        }
        // 2. double
        else if (propType == entt::resolve<double>())
        {
            QDoubleSpinBox* spin = new QDoubleSpinBox();
            spin->setRange(-100000.0, 100000.0);
            spin->setSingleStep(0.1);
            spin->setValue(propValue.cast<double>());

            connect(spin, &QDoubleSpinBox::valueChanged, [updateFieldInECS](double val) {
                updateFieldInECS(val);
                });

            formLayout->addRow(labelText, spin);
        }
        // 3. int / uint32_t
        else if (propType == entt::resolve<int>() || propType == entt::resolve<uint32_t>())
        {
            QSpinBox* spin = new QSpinBox();
            spin->setRange(-100000, 100000);
            spin->setValue(propType == entt::resolve<int>() ? propValue.cast<int>() : static_cast<int>(propValue.cast<uint32_t>()));

            connect(spin, &QSpinBox::valueChanged, [updateFieldInECS, propType](int val) {
                if (propType == entt::resolve<int>()) updateFieldInECS(val);
                else updateFieldInECS(static_cast<uint32_t>(val));
                });

            formLayout->addRow(labelText, spin);
        }
        // 4. bool
        else if (propType == entt::resolve<bool>())
        {
            QCheckBox* checkBox = new QCheckBox();
            checkBox->setChecked(propValue.cast<bool>());

            connect(checkBox, &QCheckBox::toggled, [updateFieldInECS](bool checked) {
                updateFieldInECS(checked);
                });

            formLayout->addRow(labelText, checkBox);
        }
        // 5. std::string
        else if (propType == entt::resolve<std::string>())
        {
            QLineEdit* lineEdit = new QLineEdit(QString::fromStdString(propValue.cast<std::string>()));

            connect(lineEdit, &QLineEdit::textChanged, [updateFieldInECS](const QString& text) {
                updateFieldInECS(text.toStdString());
                });

            formLayout->addRow(labelText, lineEdit);
        }
        // 6. Math::Vec3
        else if (propType == entt::resolve<Math::Vec3>())
        {
            Math::Vec3 vec = propValue.cast<Math::Vec3>();

            QWidget* vecWidget = new QWidget();
            QHBoxLayout* vecLayout = new QHBoxLayout(vecWidget);
            vecLayout->setContentsMargins(0, 0, 0, 0);

            auto createAxisSpin = [this, &vecLayout, metaData, parentType](float initialVal, const QString& axisLabel, auto axisSetter) {
                QLabel* lbl = new QLabel(axisLabel);
                lbl->setStyleSheet("color: #888; font-weight: bold;");
                vecLayout->addWidget(lbl);

                QDoubleSpinBox* spin = new QDoubleSpinBox();
                spin->setRange(-10000.0, 10000.0);
                spin->setSingleStep(0.1);
                spin->setValue(static_cast<double>(initialVal));

                connect(spin, &QDoubleSpinBox::valueChanged, [this, metaData, parentType, axisSetter](double val) {
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

            createAxisSpin(vec.x, "X", [](Math::Vec3& v, float val) { v.x = val; });
            createAxisSpin(vec.y, "Y", [](Math::Vec3& v, float val) { v.y = val; });
            createAxisSpin(vec.z, "Z", [](Math::Vec3& v, float val) { v.z = val; });

            formLayout->addRow(labelText, vecWidget);
        }
        // 7. Math::Vec4
        else if (propType == entt::resolve<Math::Vec4>())
        {
            Math::Vec4 vec = propValue.cast<Math::Vec4>();

            QWidget* vecWidget = new QWidget();
            QHBoxLayout* vecLayout = new QHBoxLayout(vecWidget);
            vecLayout->setContentsMargins(0, 0, 0, 0);

            auto createAxisSpin = [this, &vecLayout, metaData, parentType](float initialVal, const QString& axisLabel, auto axisSetter) {
                QLabel* lbl = new QLabel(axisLabel);
                lbl->setStyleSheet("color: #888; font-weight: bold;");
                vecLayout->addWidget(lbl);

                QDoubleSpinBox* spin = new QDoubleSpinBox();
                spin->setRange(-10000.0, 10000.0);
                spin->setSingleStep(0.1);
                spin->setValue(static_cast<double>(initialVal));

                connect(spin, &QDoubleSpinBox::valueChanged, [this, metaData, parentType, axisSetter](double val) {
                    if (auto getFunc = parentType.func("get"_hs); getFunc) {
                        entt::meta_any freshInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                        if (freshInstance) {
                            Math::Vec4 currentVec = metaData.get(freshInstance).cast<Math::Vec4>();
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

            createAxisSpin(vec.x, "X", [](Math::Vec4& v, float val) { v.x = val; });
            createAxisSpin(vec.y, "Y", [](Math::Vec4& v, float val) { v.y = val; });
            createAxisSpin(vec.z, "Z", [](Math::Vec4& v, float val) { v.z = val; });
            createAxisSpin(vec.w, "W", [](Math::Vec4& v, float val) { v.w = val; });

            formLayout->addRow(labelText, vecWidget);
        }
        // 8. glm::quat
        else if (propType == entt::resolve<glm::quat>())
        {
            glm::quat q = propValue.cast<glm::quat>();
            Math::Vec3 eulerDeg = glm::degrees(glm::eulerAngles(q));

            QWidget* rotWidget = new QWidget();
            QHBoxLayout* rotLayout = new QHBoxLayout(rotWidget);
            rotLayout->setContentsMargins(0, 0, 0, 0);

            auto createRotSpin = [this, &rotLayout, metaData, parentType](float initialVal, const QString& axisLabel, int axisIndex) {
                QLabel* lbl = new QLabel(axisLabel);
                lbl->setStyleSheet("color: #888; font-weight: bold;");
                rotLayout->addWidget(lbl);

                QDoubleSpinBox* spin = new QDoubleSpinBox();
                spin->setRange(-360.0, 360.0);
                spin->setSingleStep(1.0);
                spin->setValue(static_cast<double>(initialVal));

                connect(spin, &QDoubleSpinBox::valueChanged, [this, metaData, parentType, axisIndex](double val) {
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

            createRotSpin(eulerDeg.x, "X", 0);
            createRotSpin(eulerDeg.y, "Y", 1);
            createRotSpin(eulerDeg.z, "Z", 2);

            formLayout->addRow(labelText, rotWidget);
        }
        // 9. MotionType Enum
        else if (propType == entt::resolve<MotionType>())
        {
            QComboBox* combo = new QComboBox();
            combo->addItem("Static", static_cast<int>(MotionType::Static));
            combo->addItem("Kinematic", static_cast<int>(MotionType::Kinematic));
            combo->addItem("Dynamic", static_cast<int>(MotionType::Dynamic));

            MotionType currentMt = propValue.cast<MotionType>();
            combo->setCurrentIndex(combo->findData(static_cast<int>(currentMt)));

            connect(combo, &QComboBox::currentIndexChanged, [updateFieldInECS, combo](int index) {
                MotionType newMt = static_cast<MotionType>(combo->itemData(index).toInt());
                updateFieldInECS(newMt);
                });

            formLayout->addRow(labelText, combo);
        }
    }

    // =========================================================================
    // PASSO 3.C: BOTÃO "+ ADD COMPONENT" E MENU SUSPENSO DINÂMICO
    // =========================================================================
    void PropertyInspectorWidget::DrawAddComponentButton()
    {
        QPushButton* addCompBtn = new QPushButton("+ Add Component", m_containerWidget);
        addCompBtn->setStyleSheet(
            "QPushButton { background-color: #2b5b84; color: white; font-weight: bold; padding: 6px; border-radius: 3px; }"
            "QPushButton:hover { background-color: #3872a4; }"
        );

        connect(addCompBtn, &QPushButton::clicked, [this, addCompBtn]() {
            ShowAddComponentMenu(addCompBtn);
            });

        m_mainLayout->addSpacing(10);
        m_mainLayout->addWidget(addCompBtn);
    }

    void PropertyInspectorWidget::ShowAddComponentMenu(QPushButton* button)
    {
        if (m_selectedEntity == entt::null || !m_registry.valid(m_selectedEntity)) return;

        // Utiliza button->window() para definir a QMainWindow (Top-Level) como pai do QMenu
        QMenu menu(button->window());
        int availableComponentsCount = 0;

        // Varre todos os tipos registrados na reflexão do EnTT
        for (auto [typeId, metaType] : entt::resolve())
        {
            // Ignora tipos utilitários ou TagComponent
            if (metaType.id() == "TagComponent"_hs || metaType.id() == "String"_hs ||
                metaType.id() == "Vec3"_hs || metaType.id() == "Vec4"_hs ||
                metaType.id() == "Quat"_hs || metaType.id() == "MotionType"_hs)
            {
                continue;
            }

            // Testa se a entidade já possui este componente
            bool alreadyHasComponent = false;
            if (auto getFunc = metaType.func("get"_hs); getFunc)
            {
                entt::meta_any compInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                if (compInstance) {
                    alreadyHasComponent = true;
                }
            }

            // Exibe apenas os componentes que a entidade NÃO possui
            if (!alreadyHasComponent)
            {
                std::string typeName(metaType.name());
                if (typeName.empty()) continue;

                QAction* action = menu.addAction(QString::fromStdString(typeName));

                connect(action, &QAction::triggered, [this, metaType]() {
                    if (auto emplaceFunc = metaType.func("emplace"_hs); emplaceFunc) {
                        emplaceFunc.invoke({}, entt::forward_as_meta(m_registry), m_selectedEntity);
                        InspectEntity(m_selectedEntity); // Redesenha a UI
                    }
                    });

                availableComponentsCount++;
            }
        }

        if (availableComponentsCount == 0) {
            QAction* disabledAction = menu.addAction("Todos os componentes já foram adicionados.");
            disabledAction->setEnabled(false);
        }

        // Abre o menu suspenso alinhado ao botão "+ Add Component"
        menu.exec(button->mapToGlobal(QPoint(0, button->height())));
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