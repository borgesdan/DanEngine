#include <Dan/Common/Renderer/Renderer.hpp>
#include <Dan/Common/System/Window.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/Macros/Platform.hpp>
#include <cstdint>
#include <Dan/Common/IO/FileSystem.hpp>

namespace Dan {   
    Renderer::Renderer(void* nativeWindowHandle, void* nativeDisplayHandle, uint32_t width, uint32_t height, FileSystem* fs) : m_width(width), m_height(height)
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
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0);

        // 2. Criação do Uniform Global de Iluminação Direcional
        // O Shader espera um vec4 (x,y,z para direção e w para intensidade)
        m_uDirLight = bgfx::createUniform("u_dirLight", bgfx::UniformType::Vec4);

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
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
        // Garante que a tela será limpa mesmo se nada for desenhado
        bgfx::touch(0); 
    }

    void Renderer::Draw(const RenderData& renderData) const
    {
        // 1. Envia as matrizes da Câmera para a GPU
        bgfx::setViewTransform(0, &renderData.viewMatrix[0][0], &renderData.projectionMatrix[0][0]);

        // 1. Configura a Luz Global para todo o frame
        float lightData[4] = {
            renderData.dirLight.direction.x, renderData.dirLight.direction.y,
            renderData.dirLight.direction.z, renderData.dirLight.intensity
        };
        bgfx::setUniform(m_uDirLight, lightData);

        // 2. Itera sobre as entidades enviadas pela lógica do jogo
        for (const auto& cmd : renderData.drawCommands)
        {
            // Posiciona a malha no mundo
            bgfx::setTransform(&cmd.transform[0][0]);

            // Reconverte os IDs numéricos puros de volta para os handles nativos tipados do bgfx
            bgfx::VertexBufferHandle vbh = { cmd.vbh };
            bgfx::IndexBufferHandle ibh = { cmd.ibh };

            // Vincula a geometria à pipeline da GPU
            bgfx::setVertexBuffer(0, vbh);
            bgfx::setIndexBuffer(ibh);
                        
            // Habilita cor (RGB/A), escrita de profundidade (Z) e o teste de proximidade (LESS).
            // A ausência de flags de CULL garante que ambos os lados do modelo sejam desenhados.
            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS);

            bgfx::TextureHandle albedoTex = { cmd.albedoTexId };
            bgfx::UniformHandle albedoSampler = { cmd.albedoSamplerId };
            if (bgfx::isValid(albedoTex)) bgfx::setTexture(0, albedoSampler, albedoTex);

            bgfx::TextureHandle normalTex = { cmd.normalTexId };
            bgfx::UniformHandle normalSampler = { cmd.normalSamplerId };
            if (bgfx::isValid(normalTex)) bgfx::setTexture(1, normalSampler, normalTex);

            bgfx::UniformHandle propsUniform = { cmd.propsUniformId };
            if (bgfx::isValid(propsUniform)) {
                bgfx::setUniform(propsUniform, &cmd.materialProps);
            }

            bgfx::TextureHandle specTex = { cmd.specularTexId };
            bgfx::UniformHandle specSampler = { cmd.specularSamplerId };
            if (bgfx::isValid(specTex)) bgfx::setTexture(2, specSampler, specTex);

            bgfx::ProgramHandle program = { cmd.programId };
            bgfx::submit(0, program);
        }
    }

    void Renderer::EndFrame()
    {
        bgfx::frame();
    }

    void Renderer::SetViewport(uint32_t width, uint32_t height) noexcept
    {
        m_width = width;
        m_height = height;
    }

} // namespace Dan