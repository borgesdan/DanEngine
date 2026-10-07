#ifndef DAN_SYSTEM_ENGINECONTEXT_HPP
#define DAN_SYSTEM_ENGINECONTEXT_HPP

#include <cstdint>
#include "../Time/GameTime.hpp"

namespace Dan {

    // Substitui o SSystemGlobalEnvironment da CryEngine.
    // NENHUM ponteiro para subsistema (como Física ou Áudio) deve existir aqui.
    struct EngineContext
    {
        GameTime time;                      // O pacote de tempo validado
        uint64_t frameCount = 0;          // Substitui o nMainFrameID do código legado
        bool isApplicationFocused = true; // Útil para pausar simulações
    };

} // namespace Dan
#endif
