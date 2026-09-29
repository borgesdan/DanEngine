#include <Dan/Common/Log.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Dan {
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;

    void Log::Init()
    {
        // Define o padrão visual da mensagem. 
        // Exemplo do padrão abaixo: " ENGINE: Sistema inicializado"
        // %^ e %$ controlam o bloco de cores
        // %T é a hora, %n é o nome do logger, %v é a mensagem atual
        spdlog::set_pattern("%^[%T] %n: %v%$");

        // Cria um logger colorido e multi-thread (mt) para o console
        s_CoreLogger = spdlog::stdout_color_mt("ENGINE");

        // Define o nível mínimo de alerta para exibir as mensagens
        s_CoreLogger->set_level(spdlog::level::trace);
    }
}
