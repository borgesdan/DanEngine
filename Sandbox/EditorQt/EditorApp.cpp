#include "EditorApp.hpp"
#include "pch.hpp"
#include <Dan/Common/Log.hpp>

namespace Dan {

    EditorApp::EditorApp(int& argc, char** argv) : QApplication(argc, argv), m_singleInstanceLock("DanEngine_Editor_Lock")
    {
        //Metadados do QCoreApplication
        QCoreApplication::setOrganizationName("Dan");
        QCoreApplication::setOrganizationDomain("dan.com");
        QCoreApplication::setApplicationName("DANENGINE");
        QSettings::setDefaultFormat(QSettings::IniFormat);
    }

    EditorApp::~EditorApp()
    {
        // A memória de m_pEditor e m_pGameEngine é liberada automaticamente aqui!
        DAN_ENGINE_INFO("Desligando EditorApp...");
    }

    bool EditorApp::CheckIfAlreadyRunning()
    {
        // Substituto multiplataforma para o CreateMutex e GetLastError() == ERROR_ALREADY_EXISTS do CryEdit.cpp
        if (!m_singleInstanceLock.create(1))
        {
            QMessageBox::warning(nullptr, "Já em execução",
                "Running multiple instances of the Editor is not supported.\nDo you want to open a new instance anyway?");
            return false;
        }
        return true;
    }

    void EditorApp::ParseCommandLineArgs()
    {
        // Substituto moderno para CEditCommandLineInfo e CommandLineToArgvW do CryEdit.cpp
        QCommandLineParser parser;
        QCommandLineOption testOption(QStringList() << "test", "Roda o editor em modo de teste.");
        parser.addOption(testOption);

        parser.process(*this);

        if (parser.isSet(testOption)) {
            m_startupParams.bTestMode = true;
            DAN_ENGINE_INFO("Modo Teste ativado via linha de comando.");
        }
    }

    bool EditorApp::Initialize()
    {
        DAN_ENGINE_INFO("Inicializando módulos do Editor...");

        ParseCommandLineArgs();

        if (!CheckIfAlreadyRunning()) {
            return false; // Interrompe a inicialização limpidamente
        }

        // Substituto do CCryEditApp::InitGameSystem() do CryEdit.cpp
        m_pGameEngine = std::make_unique<CGameEngine>();
        // m_pGameEngine->Init(...); 

        // Aloca o gerenciador principal do editor 
        m_pEditor = std::make_unique<CEditorImpl>();
        // m_pEditor->Init();

        // Configura o IdleProcessing para rodar continuamente junto com o Qt
        QTimer* idleTimer = new QTimer(this);
        connect(idleTimer, &QTimer::timeout, this, &EditorApp::OnIdleUpdate);
        idleTimer->start(0); // 0ms = Rodar sempre que a interface não estiver travada

        return true;
    }

    void EditorApp::OnIdleUpdate()
    {
        // Substituto do CCryEditApp::IdleProcessing() do CryEdit.cpp
        // Aqui dentro o m_pGameEngine->Update() seria chamado!
    }

} // namespace Dan