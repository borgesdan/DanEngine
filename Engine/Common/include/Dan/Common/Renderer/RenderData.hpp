#pragma once

#include "../Math/MathTypes.hpp"
#include <vector>
#include <cstdint>
#include <bgfx/bgfx.h>

namespace Dan {

    struct DirectionalLightData {
        Math::Vec3 direction;
        Math::Vec3 color;
        float intensity;
    };

    // Representa um único objeto a ser desenhado na tela
    struct DrawCommand
    {
        Math::Mat4 transform;
        uint16_t vbh;
        uint16_t ibh;
        uint16_t programId;

        // Múltiplos IDs de Textura e Samplers
        uint16_t albedoTexId, albedoSamplerId;
        uint16_t normalTexId, normalSamplerId;
        uint16_t specularTexId, specularSamplerId;

        uint16_t propsUniformId;
        Math::Vec4 materialProps;
    };

    // O pacote de dados enviado da lógica do jogo para a GPU a cada frame
    struct RenderData
    {
        // Matrizes da Câmera ativa
        Math::Mat4 viewMatrix{ 1.0f };
        Math::Mat4 projectionMatrix{ 1.0f };

        // Lista de objetos que passaram pelo Frustum Culling e devem ser desenhados
        std::vector<DrawCommand> drawCommands;

        // Para simplificar, assumimos 1 luz direcional principal
        DirectionalLightData dirLight;
    };

} // namespace Dan

