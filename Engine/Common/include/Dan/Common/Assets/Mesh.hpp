#ifndef DAN_MESH_HPP
#define DAN_MESH_HPP

#include "AssetManager.hpp"
#include "../Math/MathTypes.hpp"
#include <vector>
#include <bgfx/bgfx.h>

namespace Dan {

    // Formato de vértice compatível com a GPU via bgfx
    struct Vertex {
        Math::Vec3 position;
        Math::Vec3 normal;
        Math::Vec2 uv;

        // Declaração do layout de memória para o bgfx
        static bgfx::VertexLayout GetLayout();
    };

    // O Asset de malha herda da classe base Asset definida no seu AssetManager
    class Mesh : public Asset {
    public:
        ~Mesh() override;

        // O ContentReader esperado pelo seu AssetManager::Load<T>
        static std::shared_ptr<Mesh> CreateFromBytes(const std::vector<uint8_t>& bytes, const std::string& extensionHint);

        bgfx::VertexBufferHandle GetVertexBuffer() const { return m_vbh; }
        bgfx::IndexBufferHandle GetIndexBuffer() const { return m_ibh; }

    private:
        bgfx::VertexBufferHandle m_vbh = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle m_ibh = BGFX_INVALID_HANDLE;
    };

} // namespace Dan

#endif