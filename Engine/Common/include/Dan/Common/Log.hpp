#ifndef DAN_COMMON_LOG_HPP
#define DAN_COMMON_LOG_HPP

#include <memory>
#include <spdlog/spdlog.h>
#include "Macros/Api.hpp"

namespace Dan {
	// Sistema de log que deve ser integrado e utilizado em toda a extensão do projeto, substituindo o assert padrão.
	// Contém Core (mensagens internas da engine) e Client (mensagens do jogo sendo desenvolvido nela).	
	// TODO: implementar Client posteriormente
	class DAN_API Log
	{
	public:
		//Chamado no início do bloco 'main' da aplicação
		static void Init();

		//Obtém o ponteiro interno do logger.
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
	};
}

#endif //DAN_ENGINE_COMMON_LOG_HPP

// Macros para uso interno da Engine. 
// A macro __VA_ARGS__ permite passar argumentos variáveis para formatação.
#define DAN_ENGINE_TRACE(...)    Dan::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define DAN_ENGINE_INFO(...)     Dan::Log::GetCoreLogger()->info(__VA_ARGS__)
#define DAN_ENGINE_WARN(...)     Dan::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define DAN_ENGINE_ERROR(...)    Dan::Log::GetCoreLogger()->error(__VA_ARGS__)
#define DAN_ENGINE_CRITICAL(...) Dan::Log::GetCoreLogger()->critical(__VA_ARGS__)
