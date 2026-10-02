#ifndef DAN_MATH_CAMERA_HPP
#define DAN_MATH_CAMERA_HPP
#include "MathTypes.hpp"

namespace Dan {

    class Camera
    {
    public:
        // Define o volume de visualização (Frustum)
        void SetPerspective(float fovRadians, float aspectRatio, float nearClip, float farClip)
        {
            // GLM já substitui toda a matemática manual de SetFrustum da CryEngine
            m_projectionMatrix = glm::perspective(fovRadians, aspectRatio, nearClip, farClip);
        }

        // Define a posição e direção
        void SetLookAt(const Math::Vec3& position, const Math::Vec3& target, const Math::Vec3& up = Math::Vec3(0, 1, 0))
        {
            m_viewMatrix = glm::lookAt(position, target, up);
        }

        [[nodiscard]] const Math::Mat4& GetViewMatrix() const { return m_viewMatrix; }
        [[nodiscard]] const Math::Mat4& GetProjectionMatrix() const { return m_projectionMatrix; }

        // Retorna a matriz combinada View-Projection para os shaders
        [[nodiscard]] Math::Mat4 GetViewProjection() const { return m_projectionMatrix * m_viewMatrix; }

    private:
        Math::Mat4 m_viewMatrix{ 1.0f };
        Math::Mat4 m_projectionMatrix{ 1.0f };
    };

}

#endif