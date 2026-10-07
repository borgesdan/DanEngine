#include <Dan/Common/Time/Clock.hpp>

namespace Dan {

    Clock::Clock()
    {
        Reset();
    }

    void Clock::Reset()
    {
        m_startTime = HighResClock::now();
        m_lastTime = m_startTime;
        m_gameTime = GameTime{}; // Zera todos os contadores
    }

    // Eliminados todos os try...catch desnecessários da versão Kaa/XNA original.
    void Clock::Step()
    {
        auto currentTime = HighResClock::now();

        // Calcula o DeltaTime absoluto em segundos (float)
        std::chrono::duration<float> deltaDuration = currentTime - m_lastTime;
        m_gameTime.elapsedRealTime = deltaDuration.count();

        // Atualiza os tempos totais usando precisão dupla (double) para evitar perda de precisão após horas de jogo
        std::chrono::duration<double> totalDuration = currentTime - m_startTime;
        m_gameTime.totalRealTime = totalDuration.count();

        // O ElapsedGameTime do XNA é o tempo real afetado pela escala de tempo atual.
        // Se timeScale for 0.0f (pausado), a lógica e a física do jogo não avançam.
        m_gameTime.elapsedGameTime = m_gameTime.elapsedRealTime * m_gameTime.timeScale;
        m_gameTime.totalGameTime += m_gameTime.elapsedGameTime;

        m_lastTime = currentTime;
    }

} // namespace Dan
