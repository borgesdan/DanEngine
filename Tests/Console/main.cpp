#include <Dan/Common/System/EngineCore.hpp>
#include <Dan/Common/Log.hpp>

int main(int argc, char* argv[])
{
    // 1. Inicialização do Log
    Dan::Log::Init();
    DAN_ENGINE_INFO("Iniciando projeto console de teste (SDL3 + bgfx)...");

    // 2. Instanciação e Inicialização Linear (RAII)
    Dan::EngineCore engineCore;

    // 3. Execução do Main Loop
    engineCore.Run();

    DAN_ENGINE_INFO("Teste encerrado com sucesso.");
    return 0;
}