#pragma once 

#include "../Math/MathTypes.hpp"

namespace Dan {
    struct CameraComponent
    {
        float fov = 60.0f;
        float nearClip = 0.1f;
        float farClip = 1000.0f;
        bool isPrimary = true; // Define se esta é a câmera que está renderizando a tela

        // Controle de Rotação Acumulada
        float yaw = 0.0f; // Começa olhando para o eixo -Z
        float pitch = 0.0f;
        float mouseSensitivity = 0.1f;

        // Configuração de Velocidade da Câmera (Preset do Editor)
        float baseSpeed = 10.0f;      // Velocidade base atual (unidades/segundo)
        float minSpeed = 0.5f;        // Limite mínimo de velocidade
        float maxSpeed = 150.0f;       // Limite máximo de velocidade

        // Matrizes em cache geradas a cada frame
        Math::Mat4 viewMatrix{ 1.0f };
        Math::Mat4 projectionMatrix{ 1.0f };
    };
}