#include <Dan/Common/Log.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Dan {
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        // Define o padrão visual da mensagem. 
        // Exemplo do padrão abaixo: " ENGINE: Sistema inicializado"
        // %^ e %$ controlam o bloco de cores
        // %T é a hora, %n é o nome do logger, %v é a mensagem atual
        spdlog::set_pattern("%^[%T] %n: %v%$");

        // Inicializa o logger do Core (Engine)
        s_CoreLogger = spdlog::stdout_color_mt("ENGINE");
        s_CoreLogger->set_level(spdlog::level::trace);

        // Inicializa o logger do Client (Jogo/App)
        s_ClientLogger = spdlog::stdout_color_mt("APP");
        s_ClientLogger->set_level(spdlog::level::trace);
    }
}
