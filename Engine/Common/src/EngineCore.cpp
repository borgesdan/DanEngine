#include <Dan/Common/System/EngineCore.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/System/Window.hpp>
#include <Dan/Common/Renderer/Renderer.hpp>
#include <Dan/Common/IO/FileSystem.hpp>
#include <Dan/Common/Assets/AssetManager.hpp>
#include <Dan/Common/GameFramework.hpp>

namespace Dan {

	class PhysicsSystem {
	public:
		void Update(EngineContext const& context) {}

	};

	// Recebe os parâmetros e inicializa o modo
	EngineCore::EngineCore(const EngineInitParams& params) : m_isEditorMode(params.isEditorMode)
	{
		InitializeSubsystems(params);
		m_clock.Reset(); // Mantido da implementação original[cite: 5]
	}

	EngineCore::~EngineCore()
	{
		// Ao sair de escopo, os unique_ptr destruirão os sistemas na ordem inversa de criação.
		// O "Shutdown" é automático e 100% seguro contra vazamentos.
	}

	void EngineCore::InitializeSubsystems(const EngineInitParams& params)
	{
		// 1. O FileSystem é o mais baixo nível, inicializado logo no início
		m_fileSystem = std::make_unique<FileSystem>();
		m_fileSystem->MountRoot("Assets/");

		// 2. O AssetManager é instanciado recebendo o FileSystem bruto
		m_assetManager = std::make_unique<AssetManager>(m_fileSystem.get());

		// Bifurcação Arquitetural : Editor vs Jogo
		if (!m_isEditorMode)
		{
			// Standalone: Cria a janela usando a sua abstração SDL3[cite: 5]
			m_window = std::make_unique<Window>("DanEngine", params.windowWidth, params.windowHeight);

			// O ideal agora é que o Renderer receba o Handle Nativo em vez do ponteiro da classe Window
			auto handle = m_window->GetNativeWindowHandle(); // Requer que você crie esse método na classe Window
			m_renderer = std::make_unique<Renderer>(handle.handle, handle.display, params.windowWidth, params.windowHeight, m_fileSystem.get());
		}
		else
		{
			// Editor: Janela SDL3 ignorada. Usa o Handle do Qt.
			// Aqui usamos a macro do seu Logger/Assert
			// DAN_CORE_ASSERT(params.nativeWindowHandle != nullptr, "Handle nulo no modo Editor!");
			m_renderer = std::make_unique<Renderer>(params.nativeWindowHandle, params.nativeDisplayHandle, params.windowWidth, params.windowHeight, m_fileSystem.get());
		}

		m_physics = std::make_unique<PhysicsSystem>();
		m_gameFramework = std::make_unique<GameFramework>(m_assetManager.get());
	}

	void EngineCore::Run()
	{
		// Trava de segurança: Run() não deve ser chamado no modo Editor
		// DAN_CORE_ASSERT(!m_isEditorMode, "Run() trava a thread do Qt. Use ProcessFrame() no Editor!");

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

		// 3. Processamento de Eventos (Isolado apenas para o modo Standalone)
		if (!m_isEditorMode && m_window)
		{
			m_window->PollEvents();

			if (m_window->ShouldClose()) {
				Stop();
				return;
			}
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