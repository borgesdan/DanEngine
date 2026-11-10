#pragma once
#include <QApplication>
#include <QCommandLineParser>
#include <QSharedMemory>
#include <memory>
#include <QStatusBar>
#include <QLabel>
#include <QMainWindow>

#include <Dan/Common/System/EngineCore.hpp>
#include "ViewportWidget.hpp"
#include "GizmoSystem.hpp"
#include "SceneOutlinerWidget.hpp"
#include "PropertyInspectorWidget.hpp"
#include "CreateObjectWidget.hpp"

namespace Dan {

    struct EditorStartupParams {
        bool bTestMode = false;
        bool bConsoleMode = false;
        QString levelToLoad;
    };

    class EditorApp : public QApplication
    {
        Q_OBJECT
    public:
        EditorApp(int& argc, char** argv);
        ~EditorApp() override;

        bool Initialize();
        void OnIdleUpdate();

    private:
        bool CheckIfAlreadyRunning();
        void ParseCommandLineArgs();

		// Funções auxiliares de configuração e inicialização
        void SetupMainWindowAndCentralWidget();
        void SetupEngineCore();
        void SetupDockWidgets();
        void SetupMenuBar();
        void SetupViewportInteraction();
        void SetupIdleLoop();

    private:
        EditorStartupParams m_startupParams;
        QSharedMemory m_singleInstanceLock;

        std::unique_ptr<QMainWindow> m_mainWindow;
        ViewportWidget* m_viewportWidget{ nullptr };

        // Docks mantidos para referência/sinais
        SceneOutlinerWidget* m_outlinerDock{ nullptr };
        PropertyInspectorWidget* m_inspectorDock{ nullptr };
        CreateObjectWidget* m_createObjectDock{ nullptr };

        // Engine Central e Estado do Editor
        std::unique_ptr<EngineCore> m_engineCore;
        entt::entity m_selectedEntity{ entt::null };
        GizmoSystem m_gizmoSystem;
        bgfx::ProgramHandle m_gizmoShader = BGFX_INVALID_HANDLE;

        // Barra de Status
        QLabel* m_cameraSpeedLabel{ nullptr };
        float m_lastCameraSpeed{ -1.0f };
    };

} // namespace Dan