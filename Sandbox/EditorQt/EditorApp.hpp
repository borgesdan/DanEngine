#ifndef DAN_EDITORQT_EDITORAPP_HPP
#define DAN_EDITORQT_EDITORAPP_HPP

#include <QApplication>
#include <QCommandLineParser>
#include <QSharedMemory>
#include <memory>

// Dependências arquiteturais (Stubs)
#include "EditorImpl.hpp"
#include "GameEngine.hpp"

namespace Dan {

    // Nossa estrutura moderna substituta para o CEditCommandLineInfo
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

        // O substituto para o antigo theApp.InitInstance()
        bool Initialize();

        // Executa a cada ciclo de ociosidade (Substituto do IdleProcessing)
        void OnIdleUpdate();

    private:
        bool CheckIfAlreadyRunning();
        void ParseCommandLineArgs();
        bool InitGameSystem();

    private:
        EditorStartupParams m_startupParams;
        QSharedMemory m_singleInstanceLock;

        // Propriedades essenciais convertidas para ponteiros inteligentes
        std::unique_ptr<CGameEngine> m_pGameEngine;
        std::unique_ptr<CEditorImpl> m_pEditor;
    };

} // namespace Dan

#endif