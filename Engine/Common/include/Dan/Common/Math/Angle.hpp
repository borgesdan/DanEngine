#ifndef DAN_MATH_ANGLE_HPP
#define DAN_MATH_ANGLE_HPP

#include <compare>
#include <numbers>

namespace Dan::Math {

    // O tipo armazena o ângulo internamente em radianos, o padrão nativo das APIs gráficas.
    struct Angle
    {
    private:
        float m_radians = 0.0f;

        constexpr explicit Angle(float radians) : m_radians(radians) {}

    public:
        constexpr Angle() = default;

        // Factories Estáticas
        [[nodiscard]] static constexpr Angle FromRadians(float radians) {
            return Angle(radians);
        }

        [[nodiscard]] static constexpr Angle FromDegrees(float degrees) {
            // C++20: Utiliza a constante pi nativa e type-safe
            return Angle(degrees * (std::numbers::pi_v<float> / 180.0f));
        }

        // Acesso e Conversão
        [[nodiscard]] constexpr float AsRadians() const { return m_radians; }

        [[nodiscard]] constexpr float AsDegrees() const {
            return m_radians * (180.0f / std::numbers::pi_v<float>);
        }

        [[nodiscard]] constexpr Angle Absolute() const {
            return Angle(m_radians < 0.0f ? -m_radians : m_radians);
        }

        // C++20: O operador Spaceship substitui todos os >, <, >=, <=, == e != do código original
        auto operator<=>(const Angle&) const = default;

        // Operadores Aritméticos
        [[nodiscard]] constexpr Angle operator+(Angle rhs) const { return Angle(m_radians + rhs.m_radians); }
        [[nodiscard]] constexpr Angle operator-(Angle rhs) const { return Angle(m_radians - rhs.m_radians); }
        [[nodiscard]] constexpr Angle operator*(float scalar) const { return Angle(m_radians * scalar); }
        [[nodiscard]] constexpr Angle operator/(float scalar) const { return Angle(m_radians / scalar); }
        [[nodiscard]] constexpr Angle operator-() const { return Angle(-m_radians); }

        constexpr Angle& operator+=(Angle rhs) { m_radians += rhs.m_radians; return *this; }
        constexpr Angle& operator-=(Angle rhs) { m_radians -= rhs.m_radians; return *this; }
        constexpr Angle& operator*=(float scalar) { m_radians *= scalar; return *this; }
        constexpr Angle& operator/=(float scalar) { m_radians /= scalar; return *this; }
    };

    // User-Defined Literals (UDL) modernizadas e isoladas em namespace
    namespace Literals {
        constexpr Angle operator""_rad(long double value) {
            return Angle::FromRadians(static_cast<float>(value));
        }
        constexpr Angle operator""_rad(unsigned long long value) {
            return Angle::FromRadians(static_cast<float>(value));
        }
        constexpr Angle operator""_deg(long double value) {
            return Angle::FromDegrees(static_cast<float>(value));
        }
        constexpr Angle operator""_deg(unsigned long long value) {
            return Angle::FromDegrees(static_cast<float>(value));
        }
    }

} // namespace Dan::Math
#endif
