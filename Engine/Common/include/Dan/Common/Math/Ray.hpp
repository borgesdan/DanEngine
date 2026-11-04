#pragma once

#include "MathTypes.hpp"
#include <limits>

namespace Dan::Math {

    struct Ray
    {
        Vec3 origin{ 0.0f };
        Vec3 direction{ 0.0f, 0.0f, -1.0f };

        // Converte coordenadas da viewport 2D para um raio 3D em World Space[cite: 4, 9]
        static Ray ScreenPointToRay(const Vec2& screenPos, const Vec2& viewportSize, const Mat4& viewMatrix, const Mat4& projMatrix)
        {
            // 1. Normalização para Normalized Device Coordinates (NDC) [-1, 1]
            float x = (2.0f * screenPos.x) / viewportSize.x - 1.0f;
            float y = 1.0f - (2.0f * screenPos.y) / viewportSize.y; // Inverte o eixo Y do Qt

            // 2. Desprojeta os pontos Near (z = -1) e Far (z = 1)[cite: 9]
            Mat4 invViewProj = glm::inverse(projMatrix * viewMatrix);

            // CORREÇÃO: Z-Near em 0.0f em vez de -1.0f
            Vec4 rayStartNDC(x, y, 0.0f, 1.0f);
            Vec4 rayEndNDC(x, y, 1.0f, 1.0f);

            Vec4 rayStartWorld = invViewProj * rayStartNDC;
            rayStartWorld /= rayStartWorld.w;

            Vec4 rayEndWorld = invViewProj * rayEndNDC;
            rayEndWorld /= rayEndWorld.w;

            Vec3 start(rayStartWorld);
            Vec3 end(rayEndWorld);
            Vec3 dir = glm::normalize(end - start);

            return Ray{ start, dir };
        }

        // Teste de intersecção Raio vs AABB (Slab Method)[cite: 11]
        bool IntersectsAABB(const Vec3& minBounds, const Vec3& maxBounds, float& tOut) const
        {
            float tMin = 0.0f;
            float tMax = std::numeric_limits<float>::max();

            for (int i = 0; i < 3; ++i)
            {
                if (std::abs(direction[i]) < 1e-6f)
                {
                    if (origin[i] < minBounds[i] || origin[i] > maxBounds[i])
                        return false;
                }
                else
                {
                    float invD = 1.0f / direction[i];
                    float t1 = (minBounds[i] - origin[i]) * invD;
                    float t2 = (maxBounds[i] - origin[i]) * invD;

                    if (t1 > t2) std::swap(t1, t2);

                    tMin = std::max(tMin, t1);
                    tMax = std::min(tMax, t2);

                    if (tMin > tMax) return false;
                }
            }

            tOut = tMin;
            return true;
        }
    };

} // namespace Dan::Maths