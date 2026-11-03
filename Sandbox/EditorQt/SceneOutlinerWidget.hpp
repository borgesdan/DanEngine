#pragma once

#include <QDockWidget>
#include <QTreeWidget>
#include <entt/entt.hpp>

namespace Dan {

    class SceneOutlinerWidget : public QDockWidget
    {
        Q_OBJECT
    public:
        explicit SceneOutlinerWidget(entt::registry& registry, QWidget* parent = nullptr);
        ~SceneOutlinerWidget() override = default;

        // Recarrega a árvore de entidades exibida
        void Refresh();

    signals:
        void EntitySelected(entt::entity entity);

    private:
        void OnItemSelectionChanged();

    private:
        entt::registry& m_registry;
        QTreeWidget* m_treeWidget{ nullptr };
    };

} // namespace Dan