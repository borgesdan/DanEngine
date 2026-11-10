#pragma once

#include <QDockWidget>
#include <QTreeWidget>
#include <QString>

namespace Dan {

    class CreateObjectWidget : public QDockWidget
    {
        Q_OBJECT
    public:
        explicit CreateObjectWidget(QWidget* parent = nullptr);
        ~CreateObjectWidget() override = default;

    signals:
        // Sinal emitido quando o usuário solicita a criação de um tipo/preset de objeto
        void CreateEntityRequested(const QString& presetType);

    private:
        void PopulateTree();
        void OnItemDoubleClicked(QTreeWidgetItem* item, int column);

    private:
        QTreeWidget* m_treeWidget{ nullptr };
    };

} // namespace Dan