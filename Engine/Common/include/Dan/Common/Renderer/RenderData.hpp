#pragma once

#include "../Math/MathTypes.hpp"
#include <vector>
#include <cstdint>
#include <bgfx/bgfx.h>

namespace Dan {

    // Representa um único objeto a ser desenhado na tela
    struct DrawCommand
    {
        Math::Mat4 transform; // A matriz de posição/rotação/escala do objeto
        uint16_t programId;;  // Identificador da textura/shader
        uint16_t vbh;
        uint16_t ibh;
    };

    // O pacote de dados enviado da lógica do jogo para a GPU a cada frame
    struct RenderData
    {
        // Matrizes da Câmera ativa
        Math::Mat4 viewMatrix{ 1.0f };
        Math::Mat4 projectionMatrix{ 1.0f };

        // Lista de objetos que passaram pelo Frustum Culling e devem ser desenhados
        std::vector<DrawCommand> drawCommands;
    };

} // namespace Dan

