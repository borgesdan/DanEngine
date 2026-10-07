#include "EditorApp.hpp" // Ajuste para o nome real do seu cabeçalho
#include "pch.hpp"
#include <Dan/Common/Log.hpp>
#include <QMainWindow>
#include <QTimer>
#include <QMessageBox>
#include "ViewportWidget.hpp"

namespace Dan {

    EditorApp::EditorApp(int& argc, char** argv) : QApplication(argc, argv), m_singleInstanceLock("DanEngine_Editor_Lock")
    {
        // Metadados do QCoreApplication
        QCoreApplication::setOrganizationName("Dan");
        QCoreApplication::setOrganizationDomain("dan.com");
        QCoreApplication::setApplicationName("DANENGINE");
        QSettings::setDefaultFormat(QSettings::IniFormat);
    }

    EditorApp::~EditorApp()
    {
        // A memória de m_engineCore é liberada automaticamente aqui, 
        // desligando o bgfx e os subsistemas de forma limpa!
        DAN_ENGINE_INFO("Desligando EditorApp e destruindo EngineCore...");
    }

    bool EditorApp::CheckIfAlreadyRunning()
    {
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
            return false;
        }

        // 1. Criação da Janela Principal do Editor puramente em Qt
        QMainWindow* mainWindow = new QMainWindow();
        mainWindow->setWindowTitle("DanEngine Sandbox");
        mainWindow->resize(1280, 720);

        // 2. Criação do Viewport que receberá os gráficos da Engine
        ViewportWidget* viewport = new ViewportWidget(mainWindow);
        mainWindow->setCentralWidget(viewport);

        // FORÇA a janela a ser exibida antes de inicializar a Engine.
        // Isso é crucial para que o sistema operacional crie o Handle nativo válido (HWND/XID)
        mainWindow->show();

        // 3. Configuração dos parâmetros e Inicialização da EngineCore
        EngineInitParams params;
        params.isEditorMode = true; // Informa que NÃO devemos criar uma janela SDL3
        params.nativeWindowHandle = viewport->GetNativeWindowHandle();

        // Se você adicionou o nativeDisplayHandle para Linux como discutimos anteriormente:
        // params.nativeDisplayHandle = viewport->GetNativeDisplayHandle(); 

        params.windowWidth = viewport->width();
        params.windowHeight = viewport->height();

        // Instancia a Engine injetando o contexto do Qt
        m_engineCore = std::make_unique<EngineCore>(params);

        // 4. Configura o IdleProcessing para rodar continuamente junto com o Qt
        QTimer* idleTimer = new QTimer(this);
        connect(idleTimer, &QTimer::timeout, this, &EditorApp::OnIdleUpdate);
        idleTimer->start(0); // 0ms = O evento timeout dispara sempre que o Qt não tiver nada para processar na fila de eventos

        return true;
    }

    void EditorApp::OnIdleUpdate()
    {
        // O Qt assumiu o controle da thread (via app.exec() no main).
        // Para atualizar a Engine sem travar a interface visual, chamamos o processamento de 1 frame por vez.
        if (m_engineCore)
        {
            m_engineCore->ProcessFrame();
        }
    }

} // namespace Dan