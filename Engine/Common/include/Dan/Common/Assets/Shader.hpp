#pragma once

#include "AssetManager.hpp"
#include <bgfx/bgfx.h>
#include <vector>
#include <string>

namespace Dan {

    class Shader : public Asset
    {
    public:
        ~Shader() override;

        // O ContentReader esperado pelo AssetManager::Load<T>
        static std::shared_ptr<Shader> CreateFromBytes(const std::vector<uint8_t>& bytes, const std::string& virtualPath);

        bgfx::ShaderHandle GetHandle() const { return m_handle; }

    private:
        bgfx::ShaderHandle m_handle = BGFX_INVALID_HANDLE;
    };

} // namespace Dan