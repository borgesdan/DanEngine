#pragma once

#include <Dan/Common/Math/Ray.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <entt/entt.hpp>

namespace Dan {

    class ObjectPicker
    {
    public:
        static entt::entity PickEntity(
            entt::registry& registry,
            const Math::Vec2& mousePos,
            const Math::Vec2& viewportSize)
        {
            // 1. Localiza a Câmera Primária na cena[cite: 4, 9]
            auto cameraView = registry.view<const Math::Transform, const CameraComponent>();
            const CameraComponent* primaryCam = nullptr;
            const Math::Transform* camTransform = nullptr;

            for (auto [entity, transform, camera] : cameraView.each())
            {
                if (camera.isPrimary) {
                    primaryCam = &camera;
                    camTransform = &transform;
                    break;
                }
            }

            if (!primaryCam) return entt::null;

            // 2. Calcula o Raio 3D a partir do clique[cite: 4]
            Math::Ray ray = Math::Ray::ScreenPointToRay(
                mousePos,
                viewportSize,
                primaryCam->viewMatrix,
                primaryCam->projectionMatrix
            );

            entt::entity closestEntity = entt::null;
            float minDistance = std::numeric_limits<float>::max();

            // 3. Testa intersecção com todas as entidades que possuem Transform e Mesh
            auto meshView = registry.view<const Math::Transform, const MeshComponent>();
            for (auto [entity, transform, meshComp] : meshView.each())
            {
                // AABB local padrão (Cubo unitário [-0.5, 0.5]) transformado para World Space
                Math::Vec3 localMin(-0.5f);
                Math::Vec3 localMax(0.5f);

                Math::Vec3 worldMin = transform.translation + (localMin * transform.scale);
                Math::Vec3 worldMax = transform.translation + (localMax * transform.scale);

                // Garante ordenação dos limites min/max
                Math::Vec3 actualMin = glm::min(worldMin, worldMax);
                Math::Vec3 actualMax = glm::max(worldMin, worldMax);

                float hitDistance = 0.0f;
                if (ray.IntersectsAABB(actualMin, actualMax, hitDistance))
                {
                    if (hitDistance < minDistance)
                    {
                        minDistance = hitDistance;
                        closestEntity = entity;
                    }
                }
            }

            return closestEntity;
        }
    };

} // namespace Dan