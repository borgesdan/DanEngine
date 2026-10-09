#pragma once

#include "GameTime.hpp"
#include <chrono>

namespace Dan {

    // Gerencia o tempo de jogo e a progressão dos frames
    class Clock
    {
    public:
        Clock();

        // Substitui Reset, Suspend e Resume do código antigo com uma interface mais limpa
        void Reset();
        void Step();

        // Controle de Pausa e Câmera Lenta
        void SetTimeScale(float scale) noexcept { m_gameTime.timeScale = scale; }
        [[nodiscard]] float GetTimeScale() const noexcept { return m_gameTime.timeScale; }

        [[nodiscard]] const GameTime& GetGameTime() const noexcept { return m_gameTime; }

    private:
        GameTime m_gameTime;

        // C++ nativo em vez de SDL_GetPerformanceCounter()
        using HighResClock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<HighResClock>;

        TimePoint m_startTime;
        TimePoint m_lastTime;
    };

} // namespace Dan


