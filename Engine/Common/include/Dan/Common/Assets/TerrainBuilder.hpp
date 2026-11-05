#pragma once

#include <Dan/Common/Components/TerrainComponent.hpp>
#include <Dan/Common/Assets/Mesh.hpp>
#include <memory>
#include <vector>

namespace Dan {

    class TerrainBuilder
    {
    public:
        // Gera a malha estática do bgfx (VertexBuffer e IndexBuffer) a partir dos dados do TerrainComponent
        static std::shared_ptr<Mesh> BuildMesh(TerrainComponent& terrain)
        {
            if (terrain.heightData.empty())
            {
                // Terreno plano por padrão se não houver dados de altura
                terrain.heightData.assign(terrain.width * terrain.height, 0.0f);
            }

            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            vertices.reserve(terrain.width * terrain.height);

            // 1. Geração de Vértices e UVs
            for (uint32_t z = 0; z < terrain.height; ++z)
            {
                for (uint32_t x = 0; x < terrain.width; ++x)
                {
                    float heightValue = terrain.heightData[z * terrain.width + x] * terrain.heightScale;

                    Vertex v{};
                    v.position = Math::Vec3(
                        x * terrain.gridSpacing.x,
                        heightValue,
                        z * terrain.gridSpacing.z
                    );

                    v.uv = Math::Vec2(
                        (float)x / (terrain.width - 1),
                        (float)z / (terrain.height - 1)
                    );

                    v.normal = Math::Vec3(0.0f, 1.0f, 0.0f); // Será recalculado
                    v.tangent = Math::Vec3(1.0f, 0.0f, 0.0f);

                    vertices.push_back(v);
                }
            }

            // 2. Geração de Índices (Triângulos)
            for (uint32_t z = 0; z < terrain.height - 1; ++z)
            {
                for (uint32_t x = 0; x < terrain.width - 1; ++x)
                {
                    uint32_t topLeft = z * terrain.width + x;
                    uint32_t topRight = topLeft + 1;
                    uint32_t bottomLeft = (z + 1) * terrain.width + x;
                    uint32_t bottomRight = bottomLeft + 1;

                    // Triângulo 1
                    indices.push_back(topLeft);
                    indices.push_back(bottomLeft);
                    indices.push_back(topRight);

                    // Triângulo 2
                    indices.push_back(topRight);
                    indices.push_back(bottomLeft);
                    indices.push_back(bottomRight);
                }
            }

            // 3. Recálculo Suave das Normais (Smooth Normals)
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                uint32_t i0 = indices[i];
                uint32_t i1 = indices[i + 1];
                uint32_t i2 = indices[i + 2];

                Math::Vec3 v0 = vertices[i0].position;
                Math::Vec3 v1 = vertices[i1].position;
                Math::Vec3 v2 = vertices[i2].position;

                Math::Vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

                vertices[i0].normal += normal;
                vertices[i1].normal += normal;
                vertices[i2].normal += normal;
            }

            for (auto& v : vertices)
            {
                v.normal = glm::normalize(v.normal);
            }

            // 4. Criação dos Buffers de VGPU
            return Mesh::CreateFromVertices(vertices, indices, "TerrainMesh");
        }
    };

} // namespace Dan