#ifndef DAN_SYSTEM_WINDOW_HPP
#define DAN_SYSTEM_WINDOW_HPP

#include <string_view>
#include <SDL3/SDL.h>

namespace Dan {
    class Window
    {
    public:
        // Construtor: Inicializa a SDL3 e cria a janela
        Window(std::string_view title, int w, int h);

        // Destrutor (RAII): Destrói a janela e desliga a SDL3
        ~Window();

        // Prevenção de cópia (A janela é um recurso único e não copiável)
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Move semantics (Permite transferir a posse da janela, útil com unique_ptr)
        Window(Window&& other) noexcept;
        Window& operator=(Window&& other) noexcept;

        void PollEvents();

        [[nodiscard]] bool ShouldClose() const noexcept;

        // Retorna o ponteiro nativo para ser injetado no Renderizador (Vulkan/DirectX)
        [[nodiscard]] SDL_Window* GetNativeWindow() const noexcept { return m_window; }

    private:
        SDL_Window* m_window{ nullptr };
        bool m_shouldClose{ false };
    };
}

#endif