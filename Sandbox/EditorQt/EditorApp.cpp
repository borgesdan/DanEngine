#include "EditorApp.hpp"
#include "ViewportWidget.hpp"
#include "SceneOutlinerWidget.hpp"
#include "PropertyInspectorWidget.hpp"

#include <Dan/Common/Log.hpp>
#include <Dan/Common/Scene/SceneSerializer.hpp>

#include "pch.hpp"
#include <Dan/Common/GameFramework.hpp>
#include "ObjectPicker.hpp"


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

        // 2. Configuração da Barra de Status (QStatusBar)
        QStatusBar* statusBar = mainWindow->statusBar();
        m_cameraSpeedLabel = new QLabel("Velocidade da Câmera: 10.0 u/s", mainWindow);
        statusBar->addPermanentWidget(m_cameraSpeedLabel); // Fixa o rótulo no canto direito da barra

        // 2. Viewport Central (bgfx)
        auto viewport = new ViewportWidget(mainWindow);
        mainWindow->setCentralWidget(viewport);
		m_viewportWidget = viewport; // Mantém referência para uso posterior

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

        auto vsColor = assetManager->Load<Shader>("Shaders/vs_color.bin");
        auto fsColor = assetManager->Load<Shader>("Shaders/fs_color.bin");
        // Cria o programa e guarda no EditorApp
        m_gizmoShader = bgfx::createProgram(vsColor->GetHandle(), fsColor->GetHandle(), false);

        // 4. Instanciação dos Painéis de Ferramentas (Docks)
        SceneOutlinerWidget* outlinerDock = new SceneOutlinerWidget(registry, mainWindow);
        PropertyInspectorWidget* inspectorDock = new PropertyInspectorWidget(registry, mainWindow);

        mainWindow->addDockWidget(Qt::LeftDockWidgetArea, outlinerDock);
        mainWindow->addDockWidget(Qt::RightDockWidgetArea, inspectorDock);

        // Sincroniza a seleção do Outliner com a variável central m_selectedEntity do EditorApp
        connect(outlinerDock, &SceneOutlinerWidget::EntitySelected, [this, inspectorDock](entt::entity entity) {
            m_selectedEntity = entity;
            inspectorDock->InspectEntity(entity);
            });

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

        // 5. Clique no Viewport: Tenta selecionar eixo do Gizmo primeiro; se não houver, faz Picking de objeto
        connect(viewport, &ViewportWidget::ViewportClicked,
            [this, outlinerDock, inspectorDock](const Math::Vec2& mousePos, const Math::Vec2& viewportSize)
            {
                auto& reg = m_engineCore->GetGameFramework()->GetRegistry();
                const auto& renderData = m_engineCore->GetGameFramework()->GetRenderData();

                // Gera o Raio 3D a partir do clique
                Math::Ray ray = Math::Ray::ScreenPointToRay(mousePos, viewportSize, renderData.viewMatrix, renderData.projectionMatrix);

                // Se temos uma entidade selecionada, testa se o clique acertou um eixo do Gizmo
                if (m_selectedEntity != entt::null && reg.valid(m_selectedEntity))
                {
                    if (auto* transform = reg.try_get<Math::Transform>(m_selectedEntity))
                    {
                        if (m_gizmoSystem.OnMousePress(ray, *transform))
                        {
                            // Clique atingiu o Gizmo: não altera a seleção de entidade
                            return;
                        }
                    }
                }

                // Se não clicou no Gizmo, faz o Object Picking no mundo 3D
                entt::entity hitEntity = ObjectPicker::PickEntity(reg, mousePos, viewportSize);
                m_selectedEntity = hitEntity;

                inspectorDock->InspectEntity(hitEntity);
                outlinerDock->SelectEntityInTree(hitEntity);
            });

        // Eventos de Soltura do Mouse para liberar o Gizmo
        connect(viewport, &ViewportWidget::ViewportMouseReleased, [this]() {
            m_gizmoSystem.OnMouseRelease();
            });

        // 6. Idle Processing (ProcessFrame)
        QTimer* idleTimer = new QTimer(this);
        connect(idleTimer, &QTimer::timeout, this, &EditorApp::OnIdleUpdate);
        idleTimer->start(0);


        return true;
    }

    void EditorApp::OnIdleUpdate()
    {
        if (m_engineCore)
        {
            auto& registry = m_engineCore->GetGameFramework()->GetRegistry();
            const auto& renderData = m_engineCore->GetGameFramework()->GetRenderData();
            const auto& input = m_engineCore->GetContext().input;

            // Se o Gizmo estiver sendo arrastado com o mouse, atualiza a posição do objeto em tempo real
            if (m_gizmoSystem.IsActive() && m_selectedEntity != entt::null && registry.valid(m_selectedEntity))
            {
                if (auto* transform = registry.try_get<Math::Transform>(m_selectedEntity))
                {
                    QPoint mousePos = m_viewportWidget->mapFromGlobal(QCursor::pos());
                    Math::Vec2 mPos(static_cast<float>(mousePos.x()), static_cast<float>(mousePos.y()));
                    Math::Vec2 vSize(static_cast<float>(m_viewportWidget->width()), static_cast<float>(m_viewportWidget->height()));

                    Math::Ray ray = Math::Ray::ScreenPointToRay(mPos, vSize, renderData.viewMatrix, renderData.projectionMatrix);
                    m_gizmoSystem.OnMouseDrag(ray, *transform);
                }
            }

            // Submete o desenho do Gizmo para o bgfx antes ou durante o frame
            if (m_selectedEntity != entt::null && registry.valid(m_selectedEntity))
            {
                if (auto* transform = registry.try_get<Math::Transform>(m_selectedEntity))
                {
                    m_gizmoSystem.RenderGizmo(*transform, renderData.viewMatrix, renderData.projectionMatrix, m_gizmoShader);
                }
            }

            // 1. Monitora a Câmera Primária para refletir a velocidade na UI do Qt
            auto cameraView = registry.view<const CameraComponent>();
            for (auto [entity, camera] : cameraView.each())
            {
                if (camera.isPrimary)
                {
                    // Atualiza o widget apenas se a velocidade mudou em relação ao quadro anterior
                    if (std::abs(camera.baseSpeed - m_lastCameraSpeed) > 0.001f)
                    {
                        m_lastCameraSpeed = camera.baseSpeed;
                        if (m_cameraSpeedLabel)
                        {
                            m_cameraSpeedLabel->setText(
                                QString("Cam Speed: %1 u/s").arg(camera.baseSpeed, 0, 'f', 1)
                            );
                        }
                    }
                    break;
                }
            }

            // Executa o processamento do frame (Lógica + Render)
            m_engineCore->ProcessFrame();
        }
    }

} // namespace Dan