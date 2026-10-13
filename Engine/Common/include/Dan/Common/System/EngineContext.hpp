#pragma once

#include <cstdint>
#include "../Time/GameTime.hpp"
#include "../Input/InputState.hpp"

namespace Dan {

    // Substitui o SSystemGlobalEnvironment da CryEngine.
    // NENHUM ponteiro para subsistema (como Física ou Áudio) deve existir aqui.
    struct EngineContext
    {
        GameTime time;                      // O pacote de tempo validado
        InputState input;
        uint64_t frameCount = 0;          // Substitui o nMainFrameID do código legado
        bool isApplicationFocused = true; // Útil para pausar simulações
    };

} // namespace Dan

