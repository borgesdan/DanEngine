#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>

int main(int argc, char* argv[]) {
	Dan::Log::Init();

    DAN_ENGINE_INFO("Log da Engine inicializado com sucesso.");
    DAN_ENGINE_WARN("O sistema de renderização ainda não foi implementado.");

    int modulosCarregados = 5;
    DAN_ENGINE_TRACE("Módulos de subsistemas carregados: {0}", modulosCarregados);

    // 2. Testando o CORE_ASSERT
    bool subsistemaDeVideoOk = false;

    // Isso irá disparar um Log Crítico no console e pausar a IDE exatamente nesta linha:
    DAN_CORE_ASSERT(subsistemaDeVideoOk, "Subsistema de vídeo falhou ao inicializar!");

    return 0;
}