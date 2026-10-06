#pragma once

#include <memory>
#include <bgfx/bgfx.h>
#include "RenderData.hpp"

namespace Dan {

    // Forward declaration da nossa janela SDL3
    class Window;

    // Estrutura que conterá os dados matemáticos (GLM) para desenhar
    struct RenderData;

    class Renderer
    {
    public:
        // Injeção de dependência: O renderizador precisa conhecer a janela, 
        // mas não consulta um Service Locator global (sem gEnv)
        explicit Renderer(Window* window);

        // Destrutor (RAII): Desliga o bgfx automaticamente
        ~Renderer();

        // O renderizador é um recurso único, não copiável
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        // Substitui a granularidade exposta de RenderBegin e RenderEnd do ISystem legado
        void BeginFrame();
        void Draw(const RenderData& renderData);
        void EndFrame();

    private:
        Window* m_window = nullptr;
        // bgfx::Init m_bgfxInitParams;
    };

} // namespace Dan