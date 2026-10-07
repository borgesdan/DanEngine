#include <Dan/Common/System/EngineCore.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/System/Window.hpp>
#include <Dan/Common/Renderer/Renderer.hpp>
#include <Dan/Common/IO/FileSystem.hpp>
#include <Dan/Common/Assets/AssetManager.hpp>
#include <Dan/Common/GameFramework.hpp>

namespace Dan {  
   
    class PhysicsSystem{
    public:
        void Update(EngineContext const& context) {}
        
    };    

    EngineCore::EngineCore()
    {
        InitializeSubsystems();
        m_clock.Reset();
    }

    EngineCore::~EngineCore()
    {
        // Ao sair de escopo, os unique_ptr destruirão os sistemas na ordem inversa de criação.
        // O "Shutdown" é automático e 100% seguro contra vazamentos.
    }

    void EngineCore::InitializeSubsystems()
    {
        // 1. O FileSystem é o mais baixo nível, inicializado logo no início
        m_fileSystem = std::make_unique<FileSystem>();
        m_fileSystem->MountRoot("Assets/");

        // 2. O AssetManager é instanciado recebendo o FileSystem bruto
        m_assetManager = std::make_unique<AssetManager>(m_fileSystem.get());

        m_window = std::make_unique<Window>("DanEngine", 1280, 720);
        m_renderer = std::make_unique<Renderer>(m_window.get());

        // 3. O GameFramework (e o registro ECS) recebe o AssetManager para carregar os modelos dos inimigos
        m_physics = std::make_unique<PhysicsSystem>();
        m_gameFramework = std::make_unique<GameFramework>(m_assetManager.get());
    }

    void EngineCore::Run()
    {
        m_isRunning = true;

        // Substitui a dependência da classe externa chamar DoFrame repetidamente.
        while (m_isRunning)
        {
            ProcessFrame();
        }
    }

    void EngineCore::Stop()
    {
        m_isRunning = false;
    }

    void EngineCore::ProcessFrame()
    {
        // 1. Atualiza o relógio (Substitui o bloco manual de std::chrono antigo)
        m_clock.Step();

        // 2. Alimenta o contexto que será injetado nos sistemas
        m_context.time = m_clock.GetGameTime();
        m_context.frameCount++;
        m_context.frameCount++;

        // Atualização sequencial rígida
        m_window->PollEvents();

        if (m_window->ShouldClose()) {
            Stop();
            return;
        }

        // Os sistemas consomem apenas o contexto (DeltaTime, etc)
        m_physics->Update(m_context);
        m_gameFramework->Update(m_context);

        // O equivalente moderno a RenderBegin, Render e RenderEnd
        m_renderer->BeginFrame();
        m_renderer->Draw(m_gameFramework->GetRenderData());
        m_renderer->EndFrame();
    }

} // namespace Dan