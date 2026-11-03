#include "EditorApp.hpp"
#include "ViewportWidget.hpp"
#include "SceneOutlinerWidget.hpp"
#include "PropertyInspectorWidget.hpp"

#include <Dan/Common/Log.hpp>
#include <Dan/Common/Scene/SceneSerializer.hpp>

#include "pch.hpp"
#include <Dan/Common/GameFramework.hpp>


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
        DAN_ENGINE_INFO("Inicializando subsistemas do Editor...");

        ParseCommandLineArgs();

        if (!CheckIfAlreadyRunning()) {
            return false;
        }

        // 1. Janela Principal do Editor
        QMainWindow* mainWindow = new QMainWindow();
        mainWindow->setWindowTitle("DanEngine Sandbox - Qt6 Editor");
        mainWindow->resize(1600, 900);
        mainWindow->setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

        // 2. Viewport Central (bgfx)
        auto viewport = new ViewportWidget(mainWindow);
        mainWindow->setCentralWidget(viewport);

        // FORÇA a exibição para gerar o Window Handle nativo
        mainWindow->show();

        // 3. Inicialização do Core da Engine
        EngineInitParams params;
        params.isEditorMode = true;
        params.nativeWindowHandle = viewport->GetNativeWindowHandle();
        params.windowWidth = viewport->width();
        params.windowHeight = viewport->height();

        m_engineCore = std::make_unique<EngineCore>(params);
        viewport->SetInputState(&m_engineCore->GetContext().input);

        // Obter referências da cena e sistemas do GameFramework
        auto& registry = m_engineCore->GetGameFramework()->GetRegistry();
        auto* assetManager = m_engineCore->GetAssetManager();
        auto* fileSystem = m_engineCore->GetFileSystem();

        // 4. Instanciação dos Painéis de Ferramentas (Docks)
        SceneOutlinerWidget* outlinerDock = new SceneOutlinerWidget(registry, mainWindow);
        PropertyInspectorWidget* inspectorDock = new PropertyInspectorWidget(registry, mainWindow);

        mainWindow->addDockWidget(Qt::LeftDockWidgetArea, outlinerDock);
        mainWindow->addDockWidget(Qt::RightDockWidgetArea, inspectorDock);

        // Conecta a seleção do Outliner diretamente ao Property Inspector
        connect(outlinerDock, &SceneOutlinerWidget::EntitySelected, inspectorDock, &PropertyInspectorWidget::InspectEntity);

        outlinerDock->Refresh();

        // 5. Configuração da Barra de Menus (QMenuBar + QFileDialog + SceneSerializer)
        QMenuBar* menuBar = mainWindow->menuBar();
        QMenu* fileMenu = menuBar->addMenu("&Arquivo");

        // Ação: Novo Nível
        QAction* newLevelAction = fileMenu->addAction("&Novo Nível");
        connect(newLevelAction, &QAction::triggered, [this, &registry, outlinerDock, inspectorDock]() {
            registry.clear();
            outlinerDock->Refresh();
            inspectorDock->InspectEntity(entt::null);
            DAN_ENGINE_INFO("Novo nível criado (Cena limpa).");
            });

        // Ação: Abrir Nível...
        QAction* openLevelAction = fileMenu->addAction("&Abrir Nível...");
        connect(openLevelAction, &QAction::triggered, [this, &registry, assetManager, fileSystem, outlinerDock, inspectorDock, mainWindow]() {
            QString filePath = QFileDialog::getOpenFileName(
                mainWindow,
                "Abrir Arquivo de Nível",
                "Assets/Levels",
                "Arquivos de Nível (*.json)"
            );

            if (!filePath.isEmpty()) {
                SceneSerializer serializer(registry, assetManager, fileSystem);
                std::string relativePath = std::filesystem::relative(filePath.toStdString(), fileSystem->GetRootPath()).string();

                if (serializer.Load(relativePath)) {
                    outlinerDock->Refresh();
                    inspectorDock->InspectEntity(entt::null);
                }
            }
            });

        // Ação: Salvar Nível...
        QAction* saveLevelAction = fileMenu->addAction("&Salvar Nível Como...");
        connect(saveLevelAction, &QAction::triggered, [this, &registry, assetManager, fileSystem, mainWindow]() {
            QString filePath = QFileDialog::getSaveFileName(
                mainWindow,
                "Salvar Nível Como",
                "Assets/Levels/example_saved.json",
                "Arquivos de Nível (*.json)"
            );

            if (!filePath.isEmpty()) {
                SceneSerializer serializer(registry, assetManager, fileSystem);
                std::string relativePath = std::filesystem::relative(filePath.toStdString(), fileSystem->GetRootPath()).string();
                serializer.Save(relativePath);
            }
            });

        fileMenu->addSeparator();
        QAction* exitAction = fileMenu->addAction("&Sair");
        connect(exitAction, &QAction::triggered, mainWindow, &QMainWindow::close);

        // 6. Idle Processing (ProcessFrame)
        QTimer* idleTimer = new QTimer(this);
        connect(idleTimer, &QTimer::timeout, this, &EditorApp::OnIdleUpdate);
        idleTimer->start(0);

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