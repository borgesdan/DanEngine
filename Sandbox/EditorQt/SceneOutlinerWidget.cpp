#include "SceneOutlinerWidget.hpp"
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <QVBoxLayout>

namespace Dan {

    SceneOutlinerWidget::SceneOutlinerWidget(entt::registry& registry, QWidget* parent)
        : QDockWidget("Scene Outliner", parent), m_registry(registry)
    {
        setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

        m_treeWidget = new QTreeWidget(this);
        m_treeWidget->setHeaderLabels({ "ID", "Tipo / Nome" });
        m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

        connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &SceneOutlinerWidget::OnItemSelectionChanged);

        setWidget(m_treeWidget);
    }

    void SceneOutlinerWidget::Refresh()
    {
        m_treeWidget->clear();

        auto allEntities = m_registry.view<entt::entity>();
        for (auto entityID : allEntities)
        {
            uint32_t rawID = static_cast<uint32_t>(entityID);

            // Identifica o tipo primário do componente para legibilidade
            QString entityLabel = "Entity";
            if (m_registry.all_of<CameraComponent>(entityID)) {
                entityLabel = "Main Camera";
            }
            else if (m_registry.all_of<DirectionalLightComponent>(entityID)) {
                entityLabel = "Directional Light";
            }
            else if (m_registry.all_of<MeshComponent>(entityID)) {
                entityLabel = "Mesh Object";
            }

            QTreeWidgetItem* item = new QTreeWidgetItem(m_treeWidget);
            item->setText(0, QString::number(rawID));
            item->setText(1, entityLabel);
            item->setData(0, Qt::UserRole, static_cast<uint32_t>(entityID));
        }
    }

    void SceneOutlinerWidget::OnItemSelectionChanged()
    {
        auto selectedItems = m_treeWidget->selectedItems();
        if (selectedItems.isEmpty()) {
            emit EntitySelected(entt::null);
            return;
        }

        uint32_t rawID = selectedItems.first()->data(0, Qt::UserRole).toUInt();
        emit EntitySelected(static_cast<entt::entity>(rawID));
    }

} // namespace Dan