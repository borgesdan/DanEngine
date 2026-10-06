#include <Dan/Common/System/EngineCore.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/System/Window.hpp>
#include <Dan/Common/System/Renderer.hpp>

namespace Dan {
    //TODO: remover
    class FileSystem{};   
   
    class PhysicsSystem{
    public:
        void Update(EngineContext const& context) {}
        
    };
    class GameFramework{
    public:
        void Update(EngineContext const& context) {}
        RenderData GetRenderData() { return {}; }
    };

    EngineCore::EngineCore()
    {
        InitializeSubsystems();
        m_lastFrameTime = std::chrono::high_resolution_clock::now();
    }

    EngineCore::~EngineCore()
    {
        // Ao sair de escopo, os unique_ptr destruirão os sistemas na ordem inversa de criação.
        // O "Shutdown" é automático e 100% seguro contra vazamentos.
    }

    void EngineCore::InitializeSubsystems()
    {
        // 1. FileSystem não depende de nada
        m_fileSystem = std::make_unique<FileSystem>();

        // 2. Janela precisa ser criada antes da GPU
        m_window = std::make_unique<Window>("DanEngine", 1280, 720);

        // 3. Renderizador recebe a janela via Injeção de Dependência (Não usa gEnv!)
        m_renderer = std::make_unique<Renderer>(m_window.get());

        // 4. Física etc.
        m_physics = std::make_unique<PhysicsSystem>();

        // 5. Framework do Jogo
        m_gameFramework = std::make_unique<GameFramework>();
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
        // Cálculo preciso de DeltaTime com C++ moderno
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsedTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;

        m_context.deltaTime = elapsedTime.count();
        m_context.totalTime += m_context.deltaTime;
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