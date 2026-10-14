#pragma once
#include "../Math/MathTypes.hpp"

namespace Dan {

    struct DirectionalLightComponent
    {
        Math::Vec3 color{ 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
        // A direção pode ser extraída do Transform, mas é comum ter um vetor isolado para facilitar
        Math::Vec3 direction{ 0.0f, -1.0f, 0.0f };
    };

    struct PointLightComponent
    {
        Math::Vec3 color{ 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
        float radius = 10.0f;
    };

} // namespace Dan