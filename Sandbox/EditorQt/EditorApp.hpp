#pragma once

#include <QApplication>
#include <QCommandLineParser>
#include <QSharedMemory>
#include <memory>

// Inclusão da nossa EngineCore moderna
#include <Dan/Common/System/EngineCore.hpp>
#include "ViewportWidget.hpp"
#include "GizmoSystem.hpp"


namespace Dan {

    struct EditorStartupParams {
        bool bTestMode = false;
        bool bConsoleMode = false;
        QString levelToLoad;
    };

    class EditorApp : public QApplication
    {
    public:
        EditorApp(int& argc, char** argv);
        ~EditorApp();

        bool Initialize();
        void OnIdleUpdate();

    private:
        bool CheckIfAlreadyRunning();
        void ParseCommandLineArgs();

    private:
        EditorStartupParams m_startupParams;
        QSharedMemory m_singleInstanceLock;
		ViewportWidget* m_viewportWidget{ nullptr };

        // Propriedade essencial: A engine central (Substitui CGameEngine e CEditorImpl)
        std::unique_ptr<EngineCore> m_engineCore;

        // Estado do Editor para Gizmos e Seleção
        entt::entity m_selectedEntity{ entt::null };
        GizmoSystem m_gizmoSystem;
        bgfx::ProgramHandle m_gizmoShader = BGFX_INVALID_HANDLE;
    };

} // namespace Dan

