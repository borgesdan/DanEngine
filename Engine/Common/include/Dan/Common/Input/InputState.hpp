#pragma once

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <array>
#include <cstdint>

namespace Dan {
    struct InputState
    {
        // Estado do Mouse
        float mouseDeltaX = 0.0f;
        float mouseDeltaY = 0.0f;

        // Ponteiro direto para o array interno da SDL3 ou buffer local do Qt
        const bool* keyboardState = nullptr;

        // Buffer local para quando o Qt gerencia a entrada de teclado no Editor
        std::array<bool, 512> localKeyBuffer{};

        // Armazena a máscara de botões do mouse
        uint32_t mouseButtonState = 0;

        // Inicializa o ponteiro de teclado para o buffer local do Editor
        void InitForEditor() noexcept {
            keyboardState = localKeyBuffer.data();
        }

        // Verificação contínua em O(1)
        [[nodiscard]] bool IsKeyDown(int sdlScancode) const noexcept {
            return keyboardState && (sdlScancode >= 0 && sdlScancode < 512) && keyboardState[sdlScancode];
        }

        [[nodiscard]] bool IsMouseButtonDown(int button) const noexcept {
            return (mouseButtonState & SDL_BUTTON_MASK(button)) != 0;
        }

        // Métodos de mutação chamados pelo ViewportWidget (Qt)
        void SetKeyDown(int sdlScancode, bool isDown) noexcept {
            if (sdlScancode >= 0 && sdlScancode < 512) {
                localKeyBuffer[sdlScancode] = isDown;
            }
        }

        void SetMouseButtonDown(int sdlButton, bool isDown) noexcept {
            if (isDown) {
                mouseButtonState |= SDL_BUTTON_MASK(sdlButton);
            }
            else {
                mouseButtonState &= ~SDL_BUTTON_MASK(sdlButton);
            }
        }

        // Reseta os deltas acumulados ao final de cada frame
        void ClearFrameDeltas() noexcept {
            mouseDeltaX = 0.0f;
            mouseDeltaY = 0.0f;
        }
    };
}