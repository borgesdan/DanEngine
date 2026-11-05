#pragma once

#include <Dan/Common/Math/MathTypes.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace Dan {

    struct TerrainComponent
    {
        std::string heightmapPath = ""; // Caminho relativo do arquivo (RAW 16-bit ou PNG)

        uint32_t width = 257;            // Resolução X da grade de vértices (deve ser 2^N + 1)
        uint32_t height = 257;           // Resolução Z da grade de vértices

        Math::Vec3 gridSpacing{ 1.0f, 1.0f, 1.0f }; // Dimensão da célula (X, Z) e escala de altura (Y)
        float heightScale = 100.0f;                  // Multiplicador de elevação máxima

        std::vector<float> heightData;   // Dados de altura normalizados [0.0, 1.0] na memória RAM

        uint32_t runtimeTerrainBodyID = 0xFFFFFFFF; // Handle opaco do corpo estático no Jolt Physics
        bool isInitialized = false;
    };

} // namespace Dan