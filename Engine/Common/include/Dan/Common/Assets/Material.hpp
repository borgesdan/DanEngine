#pragma once
#include "Shader.hpp"
#include <memory>

namespace Dan {

    class Material
    {
    public:
        // O Material recebe os dois shaders já carregados pelo AssetManager
        Material(std::shared_ptr<Shader> vertexShader, std::shared_ptr<Shader> fragmentShader)
            : m_vertexShader(vertexShader), m_fragmentShader(fragmentShader)
        {
            // O parâmetro 'true' avisa ao bgfx para destruir os handles de shader 
            // internamente quando o programa for destruído.
            m_program = bgfx::createProgram(m_vertexShader->GetHandle(), m_fragmentShader->GetHandle(), true);
        }

        ~Material()
        {
            if (bgfx::isValid(m_program)) {
                bgfx::destroy(m_program);
            }
        }

        bgfx::ProgramHandle GetProgram() const { return m_program; }

    private:
        std::shared_ptr<Shader> m_vertexShader;
        std::shared_ptr<Shader> m_fragmentShader;
        bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;
    };

} // namespace Dan