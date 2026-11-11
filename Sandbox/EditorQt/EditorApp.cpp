#include "EditorApp.hpp"
#include "ObjectPicker.hpp"
#include "pch.hpp"

#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/GameFramework.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Scene/SceneSerializer.hpp>
#include <Dan/Common/Scene/EntityFactory.hpp>

namespace Dan {

    EditorApp::EditorApp(int& argc, char** argv)
        : QApplication(argc, argv), m_singleInstanceLock("DanEngine_Editor_Lock")
    {
        QCoreApplication::setOrganizationName("Dan");
        QCoreApplication::setOrganizationDomain("dan.com");
        QCoreApplication::setApplicationName("DANENGINE");
        QSettings::setDefaultFormat(QSettings::IniFormat);
    }

    EditorApp::~EditorApp()
    {
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

    // =========================================================================
    // FLUXO PRINCIPAL DE INICIALIZAÇÃO
    // =========================================================================
    bool EditorApp::Initialize()
    {
        DAN_ENGINE_INFO("Inicializando subsistemas do Editor...");

        ParseCommandLineArgs();

        if (!CheckIfAlreadyRunning()) {
            return false;
        }

        SetupMainWindowAndCentralWidget();
        SetupEngineCore();
        SetupDockWidgets();
        SetupMenuBar();
        SetupViewportInteraction();
        SetupIdleLoop();

        return true;
    }

    // -------------------------------------------------------------------------
    // 1. Configuração da Janela Principal, Barra de Status e Viewport
    // -------------------------------------------------------------------------
    void EditorApp::SetupMainWindowAndCentralWidget()
    {
        m_mainWindow = std::make_unique<QMainWindow>();
        m_mainWindow->setWindowTitle("DanEngine Sandbox - Qt6 Editor");
        m_mainWindow->resize(1600, 900);
        m_mainWindow->setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

        // Barra de Status
        QStatusBar* statusBar = m_mainWindow->statusBar();
        m_cameraSpeedLabel = new QLabel("Velocidade da Câmera: 10.0 u/s", m_mainWindow.get());
        statusBar->addPermanentWidget(m_cameraSpeedLabel);

        // Viewport Central (bgfx)
        m_viewportWidget = new ViewportWidget(m_mainWindow.get());
        m_mainWindow->setCentralWidget(m_viewportWidget);

        // FORÇA a exibição para garantir que o SO instancie o Window Handle nativo
        m_mainWindow->show();
    }

    // -------------------------------------------------------------------------
    // 2. Inicialização do Core da Engine e Shaders do Editor
    // -------------------------------------------------------------------------
    void EditorApp::SetupEngineCore()
    {
        EngineInitParams params;
        params.isEditorMode = true;
        params.nativeWindowHandle = m_viewportWidget->GetNativeWindowHandle();
        params.windowWidth = m_viewportWidget->width();
        params.windowHeight = m_viewportWidget->height();

        m_engineCore = std::make_unique<EngineCore>(params);
        m_viewportWidget->SetInputState(&m_engineCore->GetContext().input);

        // Carregamento de Recursos/Shaders do Editor (Gizmo)
        auto* assetManager = m_engineCore->GetAssetManager();
        auto vsColor = assetManager->Load<Shader>("Shaders/vs_color.bin");
        auto fsColor = assetManager->Load<Shader>("Shaders/fs_color.bin");
        m_gizmoShader = bgfx::createProgram(vsColor->GetHandle(), fsColor->GetHandle(), false);
    }

    // -------------------------------------------------------------------------
    // 3. Instanciação e Conexão dos Painéis Lateral (Docks)
    // -------------------------------------------------------------------------
    void EditorApp::SetupDockWidgets()
    {
        auto& registry = m_engineCore->GetGameFramework()->GetRegistry();

        m_outlinerDock = new SceneOutlinerWidget(registry, m_mainWindow.get());
        m_inspectorDock = new PropertyInspectorWidget(registry, m_mainWindow.get());
        m_createObjectDock = new CreateObjectWidget(m_mainWindow.get());

        m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_outlinerDock);
        m_mainWindow->addDockWidget(Qt::RightDockWidgetArea, m_inspectorDock);
        m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_createObjectDock);
        m_mainWindow->tabifyDockWidget(m_outlinerDock, m_createObjectDock);

        // Conexão: Criação de Entidades via Preset
        connect(m_createObjectDock, &CreateObjectWidget::CreateEntityRequested,
            [this](const QString& presetType)
            {
                auto& registry = m_engineCore->GetGameFramework()->GetRegistry();
                auto* assetManager = m_engineCore->GetAssetManager();

                Math::Vec3 spawnPos(0.0f, 0.0f, 0.0f);
                entt::entity newEntity = entt::null;

                if (presetType == "Empty Entity") {
                    newEntity = EntityFactory::CreateEmptyEntity(registry, "Empty Entity", spawnPos);
                }
                else if (presetType == "Static Mesh Entity") {
                    newEntity = EntityFactory::CreateStaticMeshEntity(registry, assetManager, "Static Mesh", spawnPos);
                }
                else if (presetType == "Directional Light Entity") {
                    newEntity = EntityFactory::CreateDirectionalLightEntity(registry, "Directional Light");
                }
                else if (presetType == "Camera Entity") {
                    newEntity = EntityFactory::CreateCameraEntity(registry, "Camera Entity", spawnPos);
                }

                if (newEntity != entt::null) {
                    m_outlinerDock->Refresh();
                    m_outlinerDock->SelectEntityInTree(newEntity);
                    m_selectedEntity = newEntity;
                    m_inspectorDock->InspectEntity(newEntity);
                }
            });

        // Conexão: Seleção de Entidades no Outliner
        connect(m_outlinerDock, &SceneOutlinerWidget::EntitySelected, [this](entt::entity entity) {
            m_selectedEntity = entity;
            m_inspectorDock->InspectEntity(entity);
            });

        // Conexão: Renomeação da Entidade no Inspector atualiza a árvore do Outliner
        connect(m_inspectorDock, &PropertyInspectorWidget::EntityRenamed,
            [this](entt::entity entity, const QString& newName) {
                m_outlinerDock->Refresh();
                m_outlinerDock->SelectEntityInTree(entity);
            });

        m_outlinerDock->Refresh();
    }

    // -------------------------------------------------------------------------
    // 4. Configuração do Menu da Aplicação e Serialização de Níveis
    // -------------------------------------------------------------------------
    void EditorApp::SetupMenuBar()
    {
        QMenuBar* menuBar = m_mainWindow->menuBar();
        QMenu* fileMenu = menuBar->addMenu("&Arquivo");

        // Ação: Novo Nível
        QAction* newLevelAction = fileMenu->addAction("&Novo Nível");
        connect(newLevelAction, &QAction::triggered, [this]() {
            auto& reg = m_engineCore->GetGameFramework()->GetRegistry();
            reg.clear();
            m_outlinerDock->Refresh();
            m_inspectorDock->InspectEntity(entt::null);
            DAN_ENGINE_INFO("Novo nível criado (Cena limpa).");
            });

        // Ação: Abrir Nível...
        QAction* openLevelAction = fileMenu->addAction("&Abrir Nível...");
        connect(openLevelAction, &QAction::triggered, [this]() {
            auto& reg = m_engineCore->GetGameFramework()->GetRegistry();
            auto* assetManager = m_engineCore->GetAssetManager();
            auto* fileSystem = m_engineCore->GetFileSystem();

            QString filePath = QFileDialog::getOpenFileName(
                m_mainWindow.get(),
                "Abrir Arquivo de Nível",
                "Assets/Levels",
                "Arquivos de Nível (*.json)"
            );

            if (!filePath.isEmpty()) {
                SceneSerializer serializer(reg, assetManager, fileSystem);
                std::string relativePath = std::filesystem::relative(filePath.toStdString(), fileSystem->GetRootPath()).string();

                if (serializer.Load(relativePath)) {
                    m_outlinerDock->Refresh();
                    m_inspectorDock->InspectEntity(entt::null);
                }
            }
            });

        // Ação: Salvar Nível...
        QAction* saveLevelAction = fileMenu->addAction("&Salvar Nível Como...");
        connect(saveLevelAction, &QAction::triggered, [this]() {
            auto& reg = m_engineCore->GetGameFramework()->GetRegistry();
            auto* assetManager = m_engineCore->GetAssetManager();
            auto* fileSystem = m_engineCore->GetFileSystem();

            QString filePath = QFileDialog::getSaveFileName(
                m_mainWindow.get(),
                "Salvar Nível Como",
                "Assets/Levels/example_saved.json",
                "Arquivos de Nível (*.json)"
            );

            if (!filePath.isEmpty()) {
                SceneSerializer serializer(reg, assetManager, fileSystem);
                std::string relativePath = std::filesystem::relative(filePath.toStdString(), fileSystem->GetRootPath()).string();
                serializer.Save(relativePath);
            }
            });

        fileMenu->addSeparator();
        QAction* exitAction = fileMenu->addAction("&Sair");
        connect(exitAction, &QAction::triggered, m_mainWindow.get(), &QMainWindow::close);
    }

    // -------------------------------------------------------------------------
    // 5. Clique no Viewport, Raycasting, Picking e Gizmos
    // -------------------------------------------------------------------------
    void EditorApp::SetupViewportInteraction()
    {
        connect(m_viewportWidget, &ViewportWidget::ViewportClicked,
            [this](const Math::Vec2& mousePos, const Math::Vec2& viewportSize)
            {
                auto& reg = m_engineCore->GetGameFramework()->GetRegistry();
                const auto& renderData = m_engineCore->GetGameFramework()->GetRenderData();

                Math::Ray ray = Math::Ray::ScreenPointToRay(mousePos, viewportSize, renderData.viewMatrix, renderData.projectionMatrix);

                // Testa se clicou primeiro num eixo do Gizmo
                if (m_selectedEntity != entt::null && reg.valid(m_selectedEntity))
                {
                    if (auto* transform = reg.try_get<Math::Transform>(m_selectedEntity))
                    {
                        if (m_gizmoSystem.OnMousePress(ray, *transform)) {
                            return; // Manteve seleção no Gizmo
                        }
                    }
                }

                // Senão, realiza Object Picking no cenário 3D
                entt::entity hitEntity = ObjectPicker::PickEntity(reg, mousePos, viewportSize);
                m_selectedEntity = hitEntity;

                m_inspectorDock->InspectEntity(hitEntity);
                m_outlinerDock->SelectEntityInTree(hitEntity);
            });

        connect(m_viewportWidget, &ViewportWidget::ViewportMouseReleased, [this]() {
            m_gizmoSystem.OnMouseRelease();
            });
    }

    // -------------------------------------------------------------------------
    // 6. Configuração do Loop de Ociosidade (Idle Processing via QTimer)
    // -------------------------------------------------------------------------
    void EditorApp::SetupIdleLoop()
    {
        QTimer* idleTimer = new QTimer(this);
        connect(idleTimer, &QTimer::timeout, this, &EditorApp::OnIdleUpdate);
        idleTimer->start(0);
    }

    // -------------------------------------------------------------------------
    // Loop principal da engine executado a cada frame ocioso do Qt
    // -------------------------------------------------------------------------
    void EditorApp::OnIdleUpdate()
    {
        if (!m_engineCore) return;

        auto& registry = m_engineCore->GetGameFramework()->GetRegistry();
        const auto& renderData = m_engineCore->GetGameFramework()->GetRenderData();

        // Drag do Gizmo com o mouse
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

        // Renderização do Gizmo
        if (m_selectedEntity != entt::null && registry.valid(m_selectedEntity))
        {
            if (auto* transform = registry.try_get<Math::Transform>(m_selectedEntity))
            {
                m_gizmoSystem.RenderGizmo(*transform, renderData.viewMatrix, renderData.projectionMatrix, m_gizmoShader);
            }
        }

        // Atualização da UI (Velocidade da Câmera)
        auto cameraView = registry.view<const CameraComponent>();
        for (auto [entity, camera] : cameraView.each())
        {
            if (camera.isPrimary)
            {
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

        // Atualização da Lógica + Render da Engine
        m_engineCore->ProcessFrame();
    }

} // namespace Dan