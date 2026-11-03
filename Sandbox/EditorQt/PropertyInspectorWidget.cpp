#include "PropertyInspectorWidget.hpp"
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <QGroupBox>
#include <glm/gtc/quaternion.hpp>

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

        // Exibe o ID da entidade
        QLabel* idLabel = new QLabel(QString("<b>Entidade ID: %1</b>").arg(static_cast<uint32_t>(m_selectedEntity)));
        m_mainLayout->addWidget(idLabel);

        // 1. Inspeciona TransformComponent
        if (auto* transform = m_registry.try_get<Math::Transform>(m_selectedEntity)) {
            DrawTransformComponent(*transform);
        }

        // 2. Inspeciona CameraComponent
        if (auto* camera = m_registry.try_get<CameraComponent>(m_selectedEntity)) {
            QGroupBox* group = new QGroupBox("Camera Component", m_containerWidget);
            QFormLayout* form = new QFormLayout(group);

            QDoubleSpinBox* fovBox = new QDoubleSpinBox();
            fovBox->setRange(10.0, 170.0);
            fovBox->setValue(camera->fov);
            connect(fovBox, &QDoubleSpinBox::valueChanged, [camera](double val) { camera->fov = static_cast<float>(val); });

            form->addRow("FOV:", fovBox);
            m_mainLayout->addWidget(group);
        }

        // 3. Inspeciona DirectionalLightComponent
        if (auto* light = m_registry.try_get<DirectionalLightComponent>(m_selectedEntity)) {
            QGroupBox* group = new QGroupBox("Directional Light", m_containerWidget);
            QFormLayout* form = new QFormLayout(group);

            QDoubleSpinBox* intensityBox = new QDoubleSpinBox();
            intensityBox->setRange(0.0, 100.0);
            intensityBox->setValue(light->intensity);
            connect(intensityBox, &QDoubleSpinBox::valueChanged, [light](double val) { light->intensity = static_cast<float>(val); });

            form->addRow("Intensidade:", intensityBox);
            m_mainLayout->addWidget(group);
        }
    }

    void PropertyInspectorWidget::DrawTransformComponent(Math::Transform& transform)
    {
        QGroupBox* group = new QGroupBox("Transform", m_containerWidget);
        QVBoxLayout* groupLayout = new QVBoxLayout(group);

        auto createVec3Control = [](const QString& labelText, Math::Vec3& vec) -> QWidget* {
            QWidget* widget = new QWidget();
            QHBoxLayout* layout = new QHBoxLayout(widget);
            layout->setContentsMargins(0, 0, 0, 0);

            layout->addWidget(new QLabel(labelText));

            auto makeSpin = [&vec](float& val) {
                QDoubleSpinBox* spin = new QDoubleSpinBox();
                spin->setRange(-10000.0, 10000.0);
                spin->setSingleStep(0.1);
                spin->setValue(val);
                QObject::connect(spin, &QDoubleSpinBox::valueChanged, [&val](double v) { val = static_cast<float>(v); });
                return spin;
                };

            layout->addWidget(makeSpin(vec.x));
            layout->addWidget(makeSpin(vec.y));
            layout->addWidget(makeSpin(vec.z));
            return widget;
            };

        // Translation
        groupLayout->addWidget(createVec3Control("Pos", transform.translation));

        // Rotation (Euler degrees)
        Math::Vec3 eulerDeg = glm::degrees(glm::eulerAngles(transform.rotation));
        QWidget* rotWidget = createVec3Control("Rot", eulerDeg);
        groupLayout->addWidget(rotWidget);

        // Scale
        groupLayout->addWidget(createVec3Control("Scl", transform.scale));

        m_mainLayout->addWidget(group);
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