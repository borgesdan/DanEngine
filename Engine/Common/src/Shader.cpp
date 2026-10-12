#include <Dan/Common/Assets/Shader.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>

namespace Dan {

    Shader::~Shader()
    {
        if (bgfx::isValid(m_handle)) {
            bgfx::destroy(m_handle);
        }
    }

    std::shared_ptr<Shader> Shader::CreateFromBytes(const std::vector<uint8_t>& bytes, const std::string& virtualPath)
    {
        DAN_CORE_ASSERT(!bytes.empty(), "Tentativa de criar shader com buffer vazio: {0}", virtualPath);

        // Copia a memória de forma segura para o bgfx assumir a posse (como fizemos na correção da Mesh)
        const bgfx::Memory* mem = bgfx::copy(bytes.data(), bytes.size());
        bgfx::ShaderHandle handle = bgfx::createShader(mem);

        DAN_CORE_ASSERT(bgfx::isValid(handle), "Falha ao compilar o shader no bgfx: {0}", virtualPath);

        auto shaderAsset = std::make_shared<Shader>();
        shaderAsset->m_handle = handle;
        return shaderAsset;
    }

} // namespace Dan