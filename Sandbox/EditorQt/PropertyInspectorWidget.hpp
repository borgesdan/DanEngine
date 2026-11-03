#pragma once

#include <QDockWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <entt/entt.hpp>
#include <Dan/Common/Math/Transform.hpp>

namespace Dan {

    class PropertyInspectorWidget : public QDockWidget
    {
        Q_OBJECT
    public:
        explicit PropertyInspectorWidget(entt::registry& registry, QWidget* parent = nullptr);
        ~PropertyInspectorWidget() override = default;

    public slots:
        // Chamado quando o usuário seleciona uma entidade no SceneOutliner
        void InspectEntity(entt::entity entity);

    private:
        void ClearLayout();
        void DrawTransformComponent(Math::Transform& transform);

    private:
        entt::registry& m_registry;
        entt::entity m_selectedEntity{ entt::null };

        QWidget* m_containerWidget{ nullptr };
        QVBoxLayout* m_mainLayout{ nullptr };
    };

} // namespace Dan