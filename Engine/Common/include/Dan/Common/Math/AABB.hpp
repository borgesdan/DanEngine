#pragma once

#include "MathTypes.hpp" // Seu arquivo com os aliases da GLM
#include <limits>

namespace Dan::Math {
    struct AABB
    {
        Vec3 min;
        Vec3 max;

        // 1. Inicialização Segura (Estado Inválido/Reverso)
        // O min começa com o MAIOR valor possível, e o max com o MENOR valor possível.
        // Isso garante que qualquer ponto adicionado depois force a caixa a abraçá-lo.
        constexpr AABB()
            : min(std::numeric_limits<float>::max()),
            max(std::numeric_limits<float>::lowest()) {}

        constexpr AABB(const Vec3& minPoint, const Vec3& maxPoint)
            : min(minPoint), max(maxPoint) {}

        // Cria uma AABB a partir de um ponto central e suas extensões (metade do tamanho)
        static constexpr AABB FromExtents(const Vec3& center, const Vec3& extents) {
            return AABB(center - extents, center + extents);
        }

        // 2. Modificadores
        void Reset() {
            min = Vec3(std::numeric_limits<float>::max());
            max = Vec3(std::numeric_limits<float>::lowest());
        }

        // Expande a AABB para englobar um novo ponto 3D (ex: os vértices de um modelo 3D)
        void Add(const Vec3& point) {
            // O glm::min/max compara x, y e z simultaneamente
            min = glm::min(min, point);
            max = glm::max(max, point);
        }

        // Funde duas caixas delimitadoras (útil para criar a AABB de um grupo de objetos)
        void Add(const AABB& other) {
            min = glm::min(min, other.min);
            max = glm::max(max, other.max);
        }

        // 3. Consultas Geométricas (Atributo [[nodiscard]] força o uso do retorno)
        [[nodiscard]] constexpr Vec3 GetCenter() const {
            return (min + max) * 0.5f;
        }

        [[nodiscard]] constexpr Vec3 GetSize() const {
            return max - min;
        }

        [[nodiscard]] constexpr Vec3 GetExtents() const {
            return (max - min) * 0.5f; // Extents é sempre a metade do tamanho total
        }

        [[nodiscard]] float GetRadius() const {
            return glm::length(GetExtents()); // Raio da esfera que engloba esta AABB
        }

        // 4. Testes de Interseção e Colisão
        // Verifica se esta caixa colide com outra caixa
        [[nodiscard]] constexpr bool IsIntersecting(const AABB& other) const {
            return (min.x <= other.max.x && max.x >= other.min.x) &&
                (min.y <= other.max.y && max.y >= other.min.y) &&
                (min.z <= other.max.z && max.z >= other.min.z);
        }

        // Verifica se um ponto 3D está dentro desta caixa
        [[nodiscard]] constexpr bool Contains(const Vec3& point) const {
            return (point.x >= min.x && point.x <= max.x) &&
                (point.y >= min.y && point.y <= max.y) &&
                (point.z >= min.z && point.z <= max.z);
        }
    };

} // namespace Dan::Math

