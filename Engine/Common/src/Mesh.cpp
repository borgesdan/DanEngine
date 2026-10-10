#include <Dan/Common/Assets/Mesh.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Dan {

    bgfx::VertexLayout Vertex::GetLayout() {
        bgfx::VertexLayout layout;
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
        return layout;
    }

    Mesh::~Mesh() {
        if (bgfx::isValid(m_vbh)) bgfx::destroy(m_vbh);
        if (bgfx::isValid(m_ibh)) bgfx::destroy(m_ibh);
    }

    std::shared_ptr<Mesh> Mesh::CreateFromBytes(const std::vector<uint8_t>& bytes, const std::string& virtualPath) {
        Assimp::Importer importer;

        // Extrai a extensão automaticamente (ex: ".obj")
        std::string ext = std::filesystem::path(virtualPath).extension().string();

        // Flags de otimização de pipeline 3D padrão da indústria
        const unsigned int importFlags =
            aiProcess_Triangulate |           // Força polígonos a serem triângulos
            aiProcess_GenNormals |            // Calcula normais se o arquivo não tiver
            aiProcess_CalcTangentSpace |      // Necessário para Normal Mapping no futuro
            aiProcess_JoinIdenticalVertices;  // Otimiza o Index Buffer

        // Lê da memória RAM utilizando o array de bytes do FileSystem
        const aiScene* scene = importer.ReadFileFromMemory(
            bytes.data(),
            bytes.size(),
            importFlags,
            ext.c_str() // Ex: "obj" ou "fbx" para ajudar o parser da Assimp
        );

        DAN_CORE_ASSERT(scene && scene->HasMeshes(), importer.GetErrorString());

        // Para este exemplo simples, carregamos apenas a primeira malha do arquivo
        const aiMesh* ai_mesh = scene->mMeshes[0];

        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        vertices.reserve(ai_mesh->mNumVertices);

        // 1. Extração de Vértices
        for (unsigned int i = 0; i < ai_mesh->mNumVertices; ++i) {
            Vertex v{};
            v.position = { ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z };

            if (ai_mesh->HasNormals()) {
                v.normal = { ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z };
            }

            // Pega o primeiro canal de UV, se existir
            if (ai_mesh->mTextureCoords[0]) {
                v.uv = { ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y };
            }

            vertices.push_back(v);
        }

        // 2. Extração de Índices (Faces)
        for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
            const aiFace& face = ai_mesh->mFaces[i];
            DAN_CORE_ASSERT(face.mNumIndices == 3, "Faces devem ser triângulos!");
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // 3. Conversão para Buffers da GPU (bgfx)
        auto finalMesh = std::make_shared<Mesh>();

        finalMesh->m_vbh = bgfx::createVertexBuffer(
            bgfx::copy(vertices.data(), vertices.size() * sizeof(Vertex)),
            Vertex::GetLayout()
        );

        finalMesh->m_ibh = bgfx::createIndexBuffer(
            bgfx::copy(indices.data(), indices.size() * sizeof(uint16_t))
        );

        return finalMesh;
    }
}