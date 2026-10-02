#ifndef DAN_MATH_OBB_HPP
#define  DAN_MATH_OBB_HPP

#include "MathTypes.hpp"

namespace Dan::Math {

    struct OBB
    {
        Mat3 orientation{ 1.0f }; // Matriz 3x3 de rotação (Equivalente ao m33)
        Vec3 extents{ 0.5f };     // Vetor de meias-extensões (Equivalente ao h)
        Vec3 center{ 0.0f };      // Posição no mundo (Equivalente ao c)

        constexpr OBB() = default;

        constexpr OBB(const Mat3& rot, const Vec3& ext, const Vec3& pos)
            : orientation(rot), extents(ext), center(pos) {}
    };

}

#endif