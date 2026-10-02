#ifndef DAN_MATH_TRANSFORM_HPP
#define DAN_MATH_TRANSFORM_HPP

#include "MathTypes.hpp"

namespace Dan::Math {

    struct Transform
    {
        // 1. Inicialização segura por padrão (Substitui o construtor vazio legado)
        Vec3 translation{ 0.0f, 0.0f, 0.0f };
        Quat rotation = glm::identity<Quat>();
        Vec3 scale{ 1.0f, 1.0f, 1.0f };

        // Construtores
        constexpr Transform() = default;

        constexpr Transform(const Vec3& t, const Quat& r, const Vec3& s)
            : translation(t), rotation(r), scale(s) {}

        // 2. Operador de Comparação C++20
        // Substitui os antigos operator== e operator!= manuais
        bool operator==(const Transform&) const = default;

        // 3. Geração da Matriz de Transformação (Substitui o ToMatrix34 legado)
        // Combina Translação, Rotação e Escala (Ordem T * R * S é padrão no GLM/OpenGL)
        [[nodiscard]] Mat4 GetMatrix() const
        {
            Mat4 mat(1.0f);
            mat = glm::translate(mat, translation);
            mat = mat * glm::mat4_cast(rotation);
            mat = glm::scale(mat, scale);
            return mat;
        }

        // 4. Multiplicação de Transforms (Substitui a função global Multiply)
        // Permite anexar objetos (Hierarquia Parent/Child) usando a sintaxe: Transform global = parent * local;
        [[nodiscard]] Transform operator*(const Transform& child) const
        {
            return Transform{
                // A posição do filho é escalada, rotacionada pelo pai e somada à posição do pai
                translation + (rotation * (child.translation * scale)),
                // Combina as rotações
                rotation * child.rotation,
                // Combina as escalas
                scale * child.scale
            };
        }

        // 5. Hook de Serialização (Substitui o antigo archive de forma agnóstica)
        // Será preenchido futuramente quando você escolher uma biblioteca (ex: nlohmann/json ou yaml-cpp)
        /*
        template <typename Archive>
        void Serialize(Archive& archive) {
            archive("Translation", translation);
            archive("Rotation", rotation);
            archive("Scale", scale);
        }
        */
    };

} // namespace Dan::Math

#endif