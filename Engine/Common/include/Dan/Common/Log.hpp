#pragma once

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

		//Obtém o ponteiro interno do logger da Engine (Core).
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		// Obtém o ponteiro interno do logger do Jogo (Client)
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// ========================================================================
// Macros para uso interno da Engine (Core)
// ========================================================================
#define DAN_ENGINE_TRACE(...)    Dan::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define DAN_ENGINE_INFO(...)     Dan::Log::GetCoreLogger()->info(__VA_ARGS__)
#define DAN_ENGINE_WARN(...)     Dan::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define DAN_ENGINE_ERROR(...)    Dan::Log::GetCoreLogger()->error(__VA_ARGS__)
#define DAN_ENGINE_CRITICAL(...) Dan::Log::GetCoreLogger()->critical(__VA_ARGS__)

// ========================================================================
// Macros para uso do Jogo (Client)
// ========================================================================
#define DAN_CLIENT_TRACE(...)    Dan::Log::GetClientLogger()->trace(__VA_ARGS__)
#define DAN_CLIENT_INFO(...)     Dan::Log::GetClientLogger()->info(__VA_ARGS__)
#define DAN_CLIENT_WARN(...)     Dan::Log::GetClientLogger()->warn(__VA_ARGS__)
#define DAN_CLIENT_ERROR(...)    Dan::Log::GetClientLogger()->error(__VA_ARGS__)
#define DAN_CLIENT_CRITICAL(...) Dan::Log::GetClientLogger()->critical(__VA_ARGS__)


