#pragma once
#include "Shader.hpp"
#include <memory>
#include "Texture2D.hpp"

namespace Dan {

    class Material
    {
    public:
        Material(std::shared_ptr<Shader> vs, std::shared_ptr<Shader> fs)
            : m_vertexShader(vs), m_fragmentShader(fs)
        {
            m_program = bgfx::createProgram(m_vertexShader->GetHandle(), m_fragmentShader->GetHandle(), true);

            // Samplers para as texturas (Slots 0, 1 e 2 no Shader)
            m_sAlbedo = bgfx::createUniform("s_albedo", bgfx::UniformType::Sampler);
            m_sNormal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler);
            m_sSpecular = bgfx::createUniform("s_specular", bgfx::UniformType::Sampler);

            // Uniform agrupado para otimizar envio (RGB = Cor Base, W = Rugosidade)
            m_uMaterialProps = bgfx::createUniform("u_materialProps", bgfx::UniformType::Vec4);

            //texturas "default" de 1×1 para evitar que o shader amostre um handle inválido 
            //o que gera undefined behavior e pode até quebrar o draw
            uint32_t whitePixel = 0xFFFFFFFF;
            m_defaultWhite = bgfx::createTexture2D(1, 1, false, 1,
                bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_NONE, bgfx::copy(&whitePixel, 4));

            uint32_t normalPixel = 0xFF8080FF; // normal "flat" (0,0,1)
            m_defaultNormal = bgfx::createTexture2D(1, 1, false, 1,
                bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_NONE, bgfx::copy(&normalPixel, 4));
        }

        ~Material()
        {
            if (bgfx::isValid(m_program)) bgfx::destroy(m_program);
            bgfx::destroy(m_sAlbedo);
            bgfx::destroy(m_sNormal);
            bgfx::destroy(m_sSpecular);
            bgfx::destroy(m_uMaterialProps);
        }

        void SetAlbedo(std::shared_ptr<Texture2D> tex) { m_albedoTex = tex; }
        void SetNormalMap(std::shared_ptr<Texture2D> tex) { m_normalTex = tex; }
        void SetSpecularMap(std::shared_ptr<Texture2D> tex) { m_specularTex = tex; }

        void SetProperties(const Math::Vec3& baseColor, float roughness) {
            m_baseColor = baseColor;
            m_roughness = roughness;
        }

        bgfx::ProgramHandle GetProgram() const { return m_program; }
        bgfx::UniformHandle GetAlbedoSampler() const { return m_sAlbedo; }
        bgfx::UniformHandle GetNormalSampler() const { return m_sNormal; }
        bgfx::UniformHandle GetSpecularSampler() const { return m_sSpecular; }
        bgfx::UniformHandle GetPropsUniform() const { return m_uMaterialProps; }

        bgfx::TextureHandle GetAlbedoHandle() const { return m_albedoTex ? m_albedoTex->GetHandle() : m_defaultWhite; }
        bgfx::TextureHandle GetNormalHandle() const { return m_normalTex ? m_normalTex->GetHandle() : m_defaultNormal; }
        bgfx::TextureHandle GetSpecularHandle() const { return m_specularTex ? m_specularTex->GetHandle() : m_defaultWhite; }

        Math::Vec4 GetPackedProperties() const { return Math::Vec4(m_baseColor, m_roughness); }

    private:
        std::shared_ptr<Shader> m_vertexShader;
        std::shared_ptr<Shader> m_fragmentShader;

        std::shared_ptr<Texture2D> m_albedoTex;
        std::shared_ptr<Texture2D> m_normalTex;
        std::shared_ptr<Texture2D> m_specularTex;

        Math::Vec3 m_baseColor{ 1.0f, 1.0f, 1.0f };
        float m_roughness = 0.5f;

        bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle m_sAlbedo, m_sNormal, m_sSpecular, m_uMaterialProps;

        bgfx::TextureHandle m_defaultWhite{ BGFX_INVALID_HANDLE };
        bgfx::TextureHandle m_defaultNormal{ BGFX_INVALID_HANDLE };
    };

} // namespace Dan