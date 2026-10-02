#ifndef DAN_MATH_MATHTYPES_HPP
#define DAN_MATH_MATHTYPES_HPP

// Inclui os tipos base (vetores e matrizes)
#include <glm/glm.hpp>
// Inclui extensões matemáticas (quatérnios e transformações geométricas)
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Dan::Math {
	// Alias para os tipos matemáticos da engine
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;

	using Mat3 = glm::mat3;
	using Mat4 = glm::mat4;

    // Em vez de instanciar a classe CRotation, você usará quat diretamente.
    using Quat = glm::quat;

    // 1. O construtor legado CRotation(CAngles3):
    // Na GLM, você converte ângulos de Euler (em radianos) para Quatérnio assim:
    inline Quat FromEuler(const Vec3& eulerAngles) {
        return Quat(eulerAngles);
    }

    // 2. O método legado ToAngles()[cite: 16]:
    // Na GLM, extrair ângulos de Euler de um Quatérnio:
    inline Vec3 ToEuler(const Quat& q) {
        return glm::eulerAngles(q);
    }

    // 3. Os métodos legados ToMatrix33() e ToMatrix()[cite: 16]:
    // A GLM lança as matrizes de rotação diretamente a partir do Quatérnio:
    inline Mat3 ToMatrix33(const Quat& q) {
        return glm::mat3_cast(q);
    }

    inline Mat4 ToMatrix44(const Quat& q) {
        return glm::mat4_cast(q);
    }
}

#endif