#pragma once

#include <memory>
#include <chrono>
#include "EngineContext.hpp"
#include "../Time/Clock.hpp"

// Stubs dos subsistemas que o Core vai gerenciar via RAII
// Em C++20, não precisamos importar os cabeçalhos completos aqui, apenas forward declarations.
namespace Dan {
	class FileSystem;
	class Window;
	class Renderer;
	class PhysicsSystem;
	class GameFramework;
	class AssetManager;
}

// Estrutura moderna para passar configurações de inicialização
struct EngineInitParams {
	bool isEditorMode = false;
	// O HWND/Pointer gerado pelo Viewport do Qt
	void* nativeWindowHandle = nullptr;
	// O handle do display gerado pelo Viewport do Qt
	void* nativeDisplayHandle = nullptr;
	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;
};

namespace Dan {

	// Substitui a interface monolítica ISystem.
	// Foca estritamente no controle do Main Loop e propriedade dos módulos.
	class EngineCore
	{
	public:
		// O construtor agora recebe os parâmetros (com valor padrão vazio para Standalone)
		explicit EngineCore(const EngineInitParams& params = {});
		~EngineCore();

		// =========================================================
		// MODO STANDALONE (Jogo)
		// =========================================================
		// Assume o controle da thread e roda o loop infinito
		void Run();
		void Stop();

		// =========================================================
		// MODO EDITOR (Sandbox / Qt)
		// =========================================================
		// Tornado público para ser chamado manualmente via QTimer(0)
		void ProcessFrame();

		EngineContext& GetContext() noexcept { return m_context; }
		GameFramework* GetGameFramework() noexcept { return m_gameFramework.get(); }
		AssetManager* GetAssetManager() noexcept { return m_assetManager.get(); }
		FileSystem* GetFileSystem() noexcept { return m_fileSystem.get(); }

	private:
		void InitializeSubsystems(const EngineInitParams& params);

	private:
		bool m_isRunning{ false };
		bool m_isEditorMode{ false };
		EngineContext m_context;

		// O gerenciamento de tempo agora utiliza a biblioteca chrono nativa do C++11/20
		Clock m_clock;

		// =========================================================================
		// Inicialização Linear (RAII)
		// A ordem de declaração ABAIXO define a ordem de destruição pelo C++!
		// O C++ garante que o Renderer seja destruído antes da Window, evitando crashes.
		// Isso elimina a necessidade da enum ESystemGlobalState e do método Quit().
		// =========================================================================
		std::unique_ptr<FileSystem> m_fileSystem;
		// No modo Editor, este ponteiro permanecerá nullptr
		std::unique_ptr<Window> m_window;
		std::unique_ptr<Renderer> m_renderer;
		std::unique_ptr<PhysicsSystem> m_physics;
		std::unique_ptr<AssetManager> m_assetManager;

		// A camada de jogo que consumirá os subsistemas acima via Injeção de Dependência
		std::unique_ptr<GameFramework> m_gameFramework;
	};

} // namespace Dan