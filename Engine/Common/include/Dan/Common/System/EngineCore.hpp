#pragma once

#include <memory>
#include <chrono>
#include "EngineContext.hpp"

// Stubs dos subsistemas que o Core vai gerenciar via RAII
// Em C++20, não precisamos importar os cabeçalhos completos aqui, apenas forward declarations.
namespace Dan {
    class FileSystem;
    class Window;
    class Renderer;
    class PhysicsSystem;
    class GameFramework;
}

namespace Dan {

    // Substitui a interface monolítica ISystem.
    // Foca estritamente no controle do Main Loop e propriedade dos módulos.
    class EngineCore
    {
    public:
        EngineCore();
        ~EngineCore();

        // Substitui chamadas manuais para DoFrame() e Update()
        void Run();
        void Stop();

    private:
        void InitializeSubsystems();
        void ProcessFrame();

    private:
        bool m_isRunning = false;
        EngineContext m_context;

        // O gerenciamento de tempo agora utiliza a biblioteca chrono nativa do C++11/20
        std::chrono::high_resolution_clock::time_point m_lastFrameTime;

        // =========================================================================
        // Inicialização Linear (RAII)
        // A ordem de declaração ABAIXO define a ordem de destruição pelo C++!
        // O C++ garante que o Renderer seja destruído antes da Window, evitando crashes.
        // Isso elimina a necessidade da enum ESystemGlobalState e do método Quit().
        // =========================================================================
        std::unique_ptr<FileSystem> m_fileSystem;
        std::unique_ptr<Window> m_window;
        std::unique_ptr<Renderer> m_renderer;
        std::unique_ptr<PhysicsSystem> m_physics;

        // A camada de jogo que consumirá os subsistemas acima via Injeção de Dependência
        std::unique_ptr<GameFramework> m_gameFramework;
    };

} // namespace Dan