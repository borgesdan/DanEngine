#pragma once

#include <SDL3/SDL_mouse.h>

namespace Dan {
    struct InputState
    {
        // Estado do Mouse
        float mouseDeltaX = 0.0f;
        float mouseDeltaY = 0.0f;

        // Ponteiro direto para o array interno da SDL3 (Leitura em O(1))
        const bool* keyboardState = nullptr;
        // Armazena a máscara de botões
        uint32_t mouseButtonState = 0; 

        // Atalho de verificação
        [[nodiscard]] bool IsKeyDown(int sdlScancode) const noexcept {
            return keyboardState && keyboardState[sdlScancode];
        }

        [[nodiscard]] bool IsMouseButtonDown(int button) const noexcept {
            return (mouseButtonState & SDL_BUTTON_MASK(button)) != 0;
        }
    };
}