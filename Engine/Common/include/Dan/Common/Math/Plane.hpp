#ifndef DAN_MATH_PLANE_HPP
#define DAN_MATH_PLANE_HPP

#include "MathTypes.hpp"

namespace Dan::Math {

    struct Plane
    {
        // O plano padrão aponta para cima (eixo Y/Z, dependendo da sua coordenada) na origem
        Vec3 normal{ 0.0f, 1.0f, 0.0f };
        float distance = 0.0f;

        constexpr Plane() = default;

        constexpr Plane(const Vec3& n, float d)
            : normal(n), distance(d) {}

        // Constrói um plano a partir de um vetor normal e um ponto 3D ancorado nele
        constexpr Plane(const Vec3& n, const Vec3& p)
            : normal(n), distance(-glm::dot(n, p)) {}

        // Constrói um plano a partir de 3 pontos 3D (ex: os vértices de um triângulo)
        // Utiliza produto vetorial para encontrar a normal perpendicular à superfície
        Plane(const Vec3& p0, const Vec3& p1, const Vec3& p2)
        {
            normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            distance = -glm::dot(normal, p0);
        }

        // C++20: Geração automática de testes de igualdade/desigualdade membro-a-membro
        bool operator==(const Plane&) const = default;

        // Retorna a distância assinada do ponto até o plano.
        // > 0.0f: o ponto está na frente do plano.
        // < 0.0f: o ponto está atrás do plano.
        // = 0.0f: o ponto intercepta o plano perfeitamente.
        [[nodiscard]] constexpr float DistanceToPoint(const Vec3& point) const
        {
            return glm::dot(normal, point) + distance;
        }

        // Rebate um vetor direcional contra o plano (útil para reflexão de luz ou projéteis)
        [[nodiscard]] constexpr Vec3 MirrorVector(const Vec3& vec) const
        {
            return vec - normal * (2.0f * glm::dot(normal, vec));
        }

        // Espelha uma posição 3D exata para o lado oposto do plano
        [[nodiscard]] constexpr Vec3 MirrorPosition(const Vec3& pos) const
        {
            return pos - normal * (2.0f * DistanceToPoint(pos));
        }
    };

} // namespace Dan::Math

#endif