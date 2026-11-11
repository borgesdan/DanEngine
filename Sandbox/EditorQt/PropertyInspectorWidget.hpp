#pragma once

#include <QDockWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QWidget>
#include <entt/entt.hpp>

namespace Dan {

    class PropertyInspectorWidget : public QDockWidget
    {
        Q_OBJECT
    public:
        explicit PropertyInspectorWidget(entt::registry& registry, QWidget* parent = nullptr);
        ~PropertyInspectorWidget() override = default;

    signals:
        // Sinal emitido quando o nome da entidade é alterado na UI
        void EntityRenamed(entt::entity entity, const QString& newName);

    public slots:
        // Chamado sempre que o usuário seleciona uma entidade no SceneOutliner ou Viewport
        void InspectEntity(entt::entity entity);

    private:
        void ClearLayout();

        // Desenha o cabeçalho fixo "General" (ID, Name, Layer)
        void DrawGeneralHeader();

        void DrawMetaTypeComponent(entt::meta_type metaType, entt::meta_any compInstance);
        void DrawFieldControl(QFormLayout* formLayout, const std::string& fieldName, entt::meta_data metaData, entt::meta_any compInstance);

    private:
        entt::registry& m_registry;
        entt::entity m_selectedEntity{ entt::null };

        QWidget* m_containerWidget{ nullptr };
        QVBoxLayout* m_mainLayout{ nullptr };
    };

} // namespace Dan