#pragma once
#include "Shader.hpp"
#include <memory>
#include "Texture2D.hpp"

namespace Dan {

    class Material
    {
    public:
        Material(std::shared_ptr<Shader> vertexShader, std::shared_ptr<Shader> fragmentShader, std::shared_ptr<Texture2D> texture)
            : m_vertexShader(vertexShader), m_fragmentShader(fragmentShader), m_albedoTexture(texture)
        {
            m_program = bgfx::createProgram(m_vertexShader->GetHandle(), m_fragmentShader->GetHandle(), true);

            // Cria o "Sampler" que fará a ponte entre o C++ e a variável s_texColor no Shader
            m_sampler = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        }

        ~Material()
        {
            if (bgfx::isValid(m_program)) bgfx::destroy(m_program);
            if (bgfx::isValid(m_sampler)) bgfx::destroy(m_sampler);
        }

        bgfx::ProgramHandle GetProgram() const { return m_program; }
        bgfx::TextureHandle GetTextureHandle() const { return m_albedoTexture ? m_albedoTexture->GetHandle() : bgfx::TextureHandle(BGFX_INVALID_HANDLE); }
        bgfx::UniformHandle GetSamplerHandle() const { return m_sampler; }

    private:
        std::shared_ptr<Shader> m_vertexShader;
        std::shared_ptr<Shader> m_fragmentShader;
        std::shared_ptr<Texture2D> m_albedoTexture;

        bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle m_sampler = BGFX_INVALID_HANDLE;        
    };

} // namespace Dan