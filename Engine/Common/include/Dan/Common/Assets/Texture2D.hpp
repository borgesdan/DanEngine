#pragma once
#include "AssetManager.hpp"
#include <bgfx/bgfx.h>
#include <vector>
#include <string>

namespace Dan {

    class Texture2D : public Asset
    {
    public:
        ~Texture2D() override;

        static std::shared_ptr<Texture2D> CreateFromBytes(const std::vector<uint8_t>& bytes, const std::string& virtualPath);

        bgfx::TextureHandle GetHandle() const { return m_handle; }

    private:
        bgfx::TextureHandle m_handle = BGFX_INVALID_HANDLE;
    };

} // namespace Dan