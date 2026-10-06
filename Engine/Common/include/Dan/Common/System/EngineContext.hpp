#ifndef DAN_SYSTEM_ENGINECONTEXT_HPP
#define DAN_SYSTEM_ENGINECONTEXT_HPP

#include <cstdint>

namespace Dan {

    // Substitui o SSystemGlobalEnvironment da CryEngine.
    // NENHUM ponteiro para subsistema (como Física ou Áudio) deve existir aqui.
    struct EngineContext
    {
        float deltaTime = 0.0f;           // Tempo transcorrido desde o último frame (em segundos)
        float totalTime = 0.0f;           // Tempo total desde o início da aplicação
        uint64_t frameCount = 0;          // Substitui o nMainFrameID do código legado
        bool isApplicationFocused = true; // Útil para pausar simulações
    };

} // namespace Dan
#endif
