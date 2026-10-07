#ifndef DAN_GAMETIME_HPP
#define DAN_GAMETIME_HPP

namespace Dan {

    // Substitui a estrutura XNA antiga, usando floats/doubles diretos (em segundos)
    struct GameTime
    {
        // Tempo decorrido desde o início da aplicação (útil para shaders e animações globais)
        double totalRealTime = 0.0;

        // Tempo real entre o frame anterior e o atual (DeltaTime absoluto)
        float elapsedRealTime = 0.0f;

        // Tempo decorrido no mundo do jogo (pode pausar se a engine pausar)
        double totalGameTime = 0.0;

        // DeltaTime principal usado na Lógica e Física (Afetado pelo TimeScale)
        float elapsedGameTime = 0.0f;

        // Multiplicador de tempo. 1.0 = Normal, 0.5 = Slow Motion, 0.0 = Pausado
        float timeScale = 1.0f;

        [[nodiscard]] constexpr bool IsRunningSlowly() const noexcept {
            return elapsedRealTime > 0.1f; // Ex: Considera lento se o frame demorar mais de 100ms
        }
    };

} // namespace Dan

#endif
