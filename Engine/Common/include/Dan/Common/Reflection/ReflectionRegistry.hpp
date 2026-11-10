#pragma once

namespace Dan::Reflection {

    // Inicializa a reflexão estática do EnTT para todos os tipos matemáticos e componentes.
    // Deve ser chamado uma única vez na inicialização da engine (EngineCore).
    void RegisterEngineTypes();

} // namespace Dan::Reflection