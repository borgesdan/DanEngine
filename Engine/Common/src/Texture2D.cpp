#include <Dan/Common/Assets/Texture2D.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Dan {

    Texture2D::~Texture2D()
    {
        if (bgfx::isValid(m_handle)) {
            bgfx::destroy(m_handle);
        }
    }

    std::shared_ptr<Texture2D> Texture2D::CreateFromBytes(const std::vector<uint8_t>& bytes, const std::string& virtualPath)
    {
        int width, height, channels;

        // Inverte a imagem no eixo Y (padrão necessário para a maioria das APIs gráficas)
        stbi_set_flip_vertically_on_load(true);

        // Decodifica os bytes puros da RAM em um array de pixels RGBA
        stbi_uc* pixels = stbi_load_from_memory(bytes.data(), bytes.size(), &width, &height, &channels, 4);
        DAN_CORE_ASSERT(pixels, "Falha ao decodificar a imagem: {0}", virtualPath);

        // Calcula o tamanho em bytes (Largura * Altura * 4 canais RGBA)
        uint32_t imageSize = width * height * 4;

        // Copia para a memória do bgfx e cria a textura na VRAM
        const bgfx::Memory* mem = bgfx::copy(pixels, imageSize);
        bgfx::TextureHandle handle = bgfx::createTexture2D(
            uint16_t(width), uint16_t(height), false, 1,
            bgfx::TextureFormat::RGBA8, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, mem
        );

        stbi_image_free(pixels); // Libera a RAM principal, pois a GPU já tem a cópia

        auto texture = std::make_shared<Texture2D>();
        texture->m_handle = handle;
        return texture;
    }

}