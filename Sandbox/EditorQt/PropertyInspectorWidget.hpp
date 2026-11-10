#pragma once

#include <QDockWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QWidget>
#include <entt/entt.hpp>

namespace Dan {

    class PropertyInspectorWidget : public QDockWidget
    {
        Q_OBJECT
    public:
        explicit PropertyInspectorWidget(entt::registry& registry, QWidget* parent = nullptr);
        ~PropertyInspectorWidget() override = default;

    public slots:
        // Chamado sempre que o usuário seleciona uma entidade no SceneOutliner ou Viewport
        void InspectEntity(entt::entity entity);

    private:
        void ClearLayout();
        void DrawMetaTypeComponent(entt::meta_type metaType, entt::meta_any compInstance);
        void DrawFieldControl(QFormLayout* formLayout, const std::string& fieldName, entt::meta_data metaData, entt::meta_any compInstance);

    private:
        entt::registry& m_registry;
        entt::entity m_selectedEntity{ entt::null };

        QWidget* m_containerWidget{ nullptr };
        QVBoxLayout* m_mainLayout{ nullptr };
    };

} // namespace Dan