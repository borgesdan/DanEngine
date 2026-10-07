#include <Dan/Common/Renderer/Renderer.hpp>
#include <Dan/Common/System/Window.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/Macros/Platform.hpp>
#include <cstdint>

namespace Dan {

    Renderer::Renderer(void* nativeWindowHandle, void* nativeDisplayHandle, uint32_t width, uint32_t height)
    {
        DAN_CORE_ASSERT(nativeWindowHandle != nullptr, "Renderizador exige uma janela válida!");

#if DAN_PLATFORM_LINUX
        DAN_CORE_ASSERT(nativeDisplayHandle != nullptr, "Renderizador exige uma handle do display!");
#endif

        bgfx::Init initParams;
        initParams.type = bgfx::RendererType::Count; // Auto-detecta Vulkan, DX12 ou Metal
        initParams.resolution.width = width;
        initParams.resolution.height = height;
        initParams.resolution.reset = BGFX_RESET_VSYNC;

        bgfx::PlatformData pd;
        pd.nwh = nativeWindowHandle;
        pd.ndt = nativeDisplayHandle;

        initParams.platformData = pd;

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