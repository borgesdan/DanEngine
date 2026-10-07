#include <Dan/Common/Renderer/Renderer.hpp>
#include <Dan/Common/System/Window.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/Macros/Platform.hpp>
#include <cstdint>

namespace Dan {

    Renderer::Renderer(Window* window) : m_window(window)
    {
        DAN_CORE_ASSERT(m_window != nullptr, "Renderizador exige uma janela válida!");

        bgfx::Init initParams;
        initParams.type = bgfx::RendererType::Count; // Auto-detecta Vulkan, DX12 ou Metal
        initParams.resolution.width = 1280;
        initParams.resolution.height = 720;
        initParams.resolution.reset = BGFX_RESET_VSYNC;

        bgfx::PlatformData pd;
        auto props = SDL_GetWindowProperties(m_window->GetNativeWindow());

#if DAN_PLATFORM_WINDOWS
        pd.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#elif DAN_PLATFORM_LINUX
        const char* videoDriver = SDL_GetCurrentVideoDriver();

        if (SDL_strcmp(videoDriver, "x11") == 0) {
            // No X11, precisamos passar o Display (ndt) e o Window (nwh)
            pd.ndt = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
            // O Window no X11 é um número (XID), precisamos fazer o cast para void*
            pd.nwh = (void*)(uintptr_t)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        }
        else if (SDL_strcmp(videoDriver, "wayland") == 0) {
            // No Wayland, passamos o Display e a Surface
            pd.ndt = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
            pd.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
        }
#elif DAN_PLATFORM_MACOS
        pd.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif	

        if (!bgfx::init(initParams))
        {
            DAN_ENGINE_CRITICAL("Falha ao inicializar o bgfx!");
            DAN_CORE_ASSERT(false, "bgfx::init falhou");
        }

        // Configura a cor de limpeza da tela (Clear color)
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

        DAN_ENGINE_INFO("Renderizador (bgfx) inicializado com sucesso.");
    }

    Renderer::~Renderer()
    {
        bgfx::shutdown();
        DAN_ENGINE_TRACE("Renderizador desligado de forma segura.");
    }

    void Renderer::BeginFrame()
    {
        // Define a viewport para cobrir a tela inteira e limpa os buffers
        bgfx::setViewRect(0, 0, 0, 1280, 720);
        // Garante que a tela será limpa mesmo se nada for desenhado
        bgfx::touch(0); 
    }

    void Renderer::Draw(const RenderData& renderData)
    {
        // 1. Envia as matrizes da Câmera para a GPU
        bgfx::setViewTransform(0, &renderData.viewMatrix[0][0], &renderData.projectionMatrix[0][0]);

        // 2. Itera sobre as entidades enviadas pela lógica do jogo
        for (const auto& cmd : renderData.drawCommands)
        {
            // Define a posição do objeto no mundo 3D
            bgfx::setTransform(&cmd.transform[0][0]);

            // Aqui você vincularia o VertexBuffer e o IndexBuffer baseados no cmd.meshId
            // bgfx::setVertexBuffer(0, GetMesh(cmd.meshId));
            // bgfx::setIndexBuffer(GetIndices(cmd.meshId));

            // Submete o desenho para o shader (material)
            // bgfx::submit(0, GetProgram(cmd.materialId));
        }
    }

    void Renderer::EndFrame()
    {
        bgfx::frame();
    }

} // namespace Dan