#include <Dan/Common/System/Window.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/Macros/Platform.hpp>
#include <Dan/Common/Input/InputState.hpp>

namespace Dan {
    Window::Window(std::string_view title, int w, int h)
    {
        // 1. Inicializa apenas o subsistema de vídeo da SDL3
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
        {
            DAN_ENGINE_CRITICAL("Falha ao inicializar SDL_VIDEO: {0}", SDL_GetError());
            DAN_CORE_ASSERT(false, "SDL_Init falhou!");
        }

        // 2. Criação da janela multiplataforma
        // As flags da SDL3 permitem alta flexibilidade (ex: SDL_WINDOW_VULKAN)
        m_window = SDL_CreateWindow(title.data(), w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

        if (!m_window)
        {
            DAN_ENGINE_CRITICAL("Falha ao criar a janela SDL3: {0}", SDL_GetError());
            DAN_CORE_ASSERT(false, "SDL_CreateWindow falhou!");
        }

        DAN_ENGINE_INFO("Janela '{0}' ({1}x{2}) criada com sucesso via SDL3.", title, w, h);
    }

    Window::~Window()
    {
        if (m_window)
        {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
            DAN_ENGINE_TRACE("Janela nativa destruída.");
        }

        // Desliga o subsistema de forma limpa
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    // Implementação de move semantics
    Window::Window(Window&& other) noexcept
        : m_window(other.m_window), m_shouldClose(other.m_shouldClose)
    {
        other.m_window = nullptr;
    }

    Window& Window::operator=(Window&& other) noexcept
    {
        if (this != &other) {
            if (m_window) {
                SDL_DestroyWindow(m_window);
            }
            m_window = other.m_window;
            m_shouldClose = other.m_shouldClose;
            other.m_window = nullptr;
        }
        return *this;
    }

    void Window::PollEvents(InputState& outInput)
    {
        // Reseta os deltas a cada frame
        outInput.mouseDeltaX = 0.0f;
        outInput.mouseDeltaY = 0.0f;

        SDL_Event event;

        // Esvazia a fila de eventos do sistema operacional
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                m_shouldClose = true;
                break;

            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                // Tratar redimensionamento (ex: atualizar Viewport do Vulkan/DirectX)
                // int width = event.window.data1;
                // int height = event.window.data2;
                break;

            case SDL_EVENT_WINDOW_MINIMIZED:
                // Opcional: Sinalizar para a EngineCore pausar o loop de renderização pesada
                break;

            case SDL_EVENT_WINDOW_RESTORED:
                // Opcional: Sinalizar para a EngineCore retomar a renderização
                break;

            case SDL_EVENT_MOUSE_MOTION:
                outInput.mouseDeltaX = event.motion.xrel;
                outInput.mouseDeltaY = event.motion.yrel;
                break;

            default:
                // Ignora eventos não mapeados (teclado, mouse, etc) por enquanto
                break;
            }
        }

        int numKeys;
        outInput.keyboardState = SDL_GetKeyboardState(&numKeys);

        // Captura o estado global do mouse (botões pressionados neste frame)
        float mouseX, mouseY;
        outInput.mouseButtonState = SDL_GetMouseState(&mouseX, &mouseY);
    }

    bool Window::ShouldClose() const noexcept
    {
        return m_shouldClose;
    }

    WindowNativeHandle Window::GetNativeWindowHandle() const {
        WindowNativeHandle nHandle;

        auto props = SDL_GetWindowProperties(m_window);

#if DAN_PLATFORM_WINDOWS
        nHandle.handle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#elif DAN_PLATFORM_LINUX
        const char* videoDriver = SDL_GetCurrentVideoDriver();

        if (SDL_strcmp(videoDriver, "x11") == 0) {
            // No X11, precisamos passar o Display (ndt) e o Window (nwh)
            nHandle.display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
            // O Window no X11 é um número (XID), precisamos fazer o cast para void*
            nHandle.handle = (void*)(uintptr_t)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        }
        else if (SDL_strcmp(videoDriver, "wayland") == 0) {
            // No Wayland, passamos o Display e a Surface
            nHandle.display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
            nHandle.handle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
        }
#elif DAN_PLATFORM_MACOS
        nHandle.handle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif	

        return nHandle;
    }
}