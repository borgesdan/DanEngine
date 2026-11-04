#include "GizmoSystem.hpp"
#include <glm/gtx/intersect.hpp>

namespace Dan {

    void GizmoSystem::RenderGizmo(const Math::Transform& targetTransform, const Math::Mat4& viewMatrix, const Math::Mat4& projMatrix, bgfx::ProgramHandle shaderProgram)
    {
        // Define a matriz de transformação do Gizmo na posição do objeto[cite: 5]
        Math::Mat4 gizmoMatrix = glm::translate(Math::Mat4(1.0f), targetTransform.translation);
        bgfx::setTransform(&gizmoMatrix[0][0]);

        // Estrutura simples de Vértices para desenhar as linhas dos 3 eixos via bgfx[cite: 4, 5]
        struct LineVertex {
            float x, y, z;
            uint32_t abgr;
        };

        static bgfx::VertexLayout layout;
        static bool layoutInitialized = false;
        if (!layoutInitialized) {
            layout.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .end();
            layoutInitialized = true;
        }

        const float axisLength = 1.5f;
        LineVertex vertices[] = {
            // Eixo X (Vermelho)
            { 0.0f, 0.0f, 0.0f, 0xFF0000FF },
            { axisLength, 0.0f, 0.0f, 0xFF0000FF },
            // Eixo Y (Verde)
            { 0.0f, 0.0f, 0.0f, 0xFF00FF00 },
            { 0.0f, axisLength, 0.0f, 0xFF00FF00 },
            // Eixo Z (Azul)
            { 0.0f, 0.0f, 0.0f, 0xFFFF0000 },
            { 0.0f, 0.0f, axisLength, 0xFFFF0000 }
        };

        bgfx::TransientVertexBuffer tvb;

        // 1. Verifica se a GPU/bgfx possui espaço para 6 vértices com o layout especificado
        if (bgfx::getAvailTransientVertexBuffer(6, layout) == 6)
        {
            // 2. Aloca a memória transiente (retorna void)
            bgfx::allocTransientVertexBuffer(&tvb, 6, layout);

            // 3. Copia os vértices e submete para renderização
            std::memcpy(tvb.data, vertices, sizeof(vertices));
            bgfx::setVertexBuffer(0, &tvb);
            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_PT_LINES | BGFX_STATE_LINEAA | BGFX_STATE_DEPTH_TEST_ALWAYS);
            bgfx::submit(0, shaderProgram);
        }
    }

    bool GizmoSystem::OnMousePress(const Math::Ray& ray, const Math::Transform& targetTransform)
    {
        const float axisRadius = 0.15f;
        const float axisLength = 1.5f;
        Math::Vec3 origin = targetTransform.translation;

        // Teste de proximidade com o Eixo X
        float tX = 0.0f;
        Math::Vec3 minX = origin;
        Math::Vec3 maxX = origin + Math::Vec3(axisLength, axisRadius, axisRadius);
        if (ray.IntersectsAABB(minX, maxX, tX)) {
            m_activeAxis = GizmoAxis::AxisX;
            m_dragPlaneNormal = Math::Vec3(0.0f, 1.0f, 0.0f);
        }
        // Teste de proximidade com o Eixo Y
        else if (ray.IntersectsAABB(origin, origin + Math::Vec3(axisRadius, axisLength, axisRadius), tX)) {
            m_activeAxis = GizmoAxis::AxisY;
            m_dragPlaneNormal = Math::Vec3(0.0f, 0.0f, 1.0f);
        }
        // Teste de proximidade com o Eixo Z
        else if (ray.IntersectsAABB(origin, origin + Math::Vec3(axisRadius, axisRadius, axisLength), tX)) {
            m_activeAxis = GizmoAxis::AxisZ;
            m_dragPlaneNormal = Math::Vec3(0.0f, 1.0f, 0.0f);
        }
        else {
            m_activeAxis = GizmoAxis::None;
            return false;
        }

        m_initialEntityPos = targetTransform.translation;
        return true;
    }

    void GizmoSystem::OnMouseDrag(const Math::Ray& ray, Math::Transform& targetTransform)
    {
        if (m_activeAxis == GizmoAxis::None) return;

        float distance = 0.0f;
        if (glm::intersectRayPlane(ray.origin, ray.direction, m_initialEntityPos, m_dragPlaneNormal, distance))
        {
            Math::Vec3 hitPoint = ray.origin + ray.direction * distance;
            Math::Vec3 delta = hitPoint - m_initialEntityPos;

            switch (m_activeAxis)
            {
            case GizmoAxis::AxisX:
                targetTransform.translation.x = m_initialEntityPos.x + delta.x;
                break;
            case GizmoAxis::AxisY:
                targetTransform.translation.y = m_initialEntityPos.y + delta.y;
                break;
            case GizmoAxis::AxisZ:
                targetTransform.translation.z = m_initialEntityPos.z + delta.z;
                break;
            default:
                break;
            }
        }
    }

    void GizmoSystem::OnMouseRelease()
    {
        m_activeAxis = GizmoAxis::None;
    }

} // namespace Dan