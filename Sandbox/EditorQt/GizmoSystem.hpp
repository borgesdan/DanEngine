#pragma once

#include <Dan/Common/Math/MathTypes.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Math/Ray.hpp>
#include <entt/entt.hpp>
#include <bgfx/bgfx.h>

namespace Dan {

    enum class GizmoAxis {
        None,
        AxisX,
        AxisY,
        AxisZ
    };

    class GizmoSystem
    {
    public:
        GizmoSystem() = default;

        // Renderiza os eixos de manipulação na posição da entidade selecionada
        void RenderGizmo(const Math::Transform& targetTransform, const Math::Mat4& viewMatrix, const Math::Mat4& projMatrix, bgfx::ProgramHandle shaderProgram);

        // Processa o clique/drag do mouse nos eixos
        bool OnMousePress(const Math::Ray& ray, const Math::Transform& targetTransform);
        void OnMouseDrag(const Math::Ray& ray, Math::Transform& targetTransform);
        void OnMouseRelease();

        [[nodiscard]] bool IsActive() const noexcept { return m_activeAxis != GizmoAxis::None; }

    private:
        GizmoAxis m_activeAxis{ GizmoAxis::None };
        Math::Vec3 m_initialEntityPos{ 0.0f };
        Math::Vec3 m_dragPlaneNormal{ 0.0f };
        Math::Vec3 m_intersectionOffset{ 0.0f };
    };

} // namespace Dan