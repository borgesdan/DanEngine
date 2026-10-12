#pragma once 

#include "../Math/MathTypes.hpp"

namespace Dan {
    struct CameraComponent
    {
        float fov = 60.0f;
        float nearClip = 0.1f;
        float farClip = 1000.0f;
        bool isPrimary = true; // Define se esta é a câmera que está renderizando a tela

        // Matrizes em cache geradas a cada frame
        Math::Mat4 viewMatrix{ 1.0f };
        Math::Mat4 projectionMatrix{ 1.0f };
    };
}