#include "CreateObjectWidget.hpp"
#include <QVBoxLayout>
#include <QPushButton>

namespace Dan {

    CreateObjectWidget::CreateObjectWidget(QWidget* parent)
        : QDockWidget("Create Object", parent)
    {
        setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

        QWidget* container = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout(container);

        m_treeWidget = new QTreeWidget(container);
        m_treeWidget->setHeaderLabels({ "Categoria / Tipo de Objeto" });
        m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

        PopulateTree();

        // Conecta o duplo clique na árvore para solicitar criação
        connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &CreateObjectWidget::OnItemDoubleClicked);

        // Botão de criação manual no rodapé do painel
        QPushButton* createBtn = new QPushButton("Criar Objeto Selecionado", container);
        connect(createBtn, &QPushButton::clicked, [this]() {
            auto items = m_treeWidget->selectedItems();
            if (!items.isEmpty()) {
                OnItemDoubleClicked(items.first(), 0);
            }
            });

        layout->addWidget(m_treeWidget);
        layout->addWidget(createBtn);

        setWidget(container);
    }

    void CreateObjectWidget::PopulateTree()
    {
        m_treeWidget->clear();

        // Categoria: General
        QTreeWidgetItem* generalCat = new QTreeWidgetItem(m_treeWidget, { "General" });
        QTreeWidgetItem* emptyEntity = new QTreeWidgetItem(generalCat, { "Empty Entity" });
        emptyEntity->setData(0, Qt::UserRole, "Empty Entity");

        // Categoria: Geometry
        QTreeWidgetItem* geometryCat = new QTreeWidgetItem(m_treeWidget, { "Geometry" });
        QTreeWidgetItem* staticMesh = new QTreeWidgetItem(geometryCat, { "Static Mesh Entity" });
        staticMesh->setData(0, Qt::UserRole, "Static Mesh Entity");

        // Categoria: Lights
        QTreeWidgetItem* lightsCat = new QTreeWidgetItem(m_treeWidget, { "Lights" });
        QTreeWidgetItem* dirLight = new QTreeWidgetItem(lightsCat, { "Directional Light Entity" });
        dirLight->setData(0, Qt::UserRole, "Directional Light Entity");

        // Categoria: Cameras
        QTreeWidgetItem* camerasCat = new QTreeWidgetItem(m_treeWidget, { "Cameras" });
        QTreeWidgetItem* cameraEnt = new QTreeWidgetItem(camerasCat, { "Camera Entity" });
        cameraEnt->setData(0, Qt::UserRole, "Camera Entity");

        m_treeWidget->expandAll();
    }

    void CreateObjectWidget::OnItemDoubleClicked(QTreeWidgetItem* item, int column)
    {
        if (!item) return;

        QString presetType = item->data(0, Qt::UserRole).toString();
        if (!presetType.isEmpty()) {
            emit CreateEntityRequested(presetType);
        }
    }

} // namespace Dan