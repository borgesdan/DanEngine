#include <Dan/Common/Scene/EntityFactory.hpp>
#include <Dan/Common/Components/TagComponent.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <Dan/Common/Assets/AssetManager.hpp>

namespace Dan {

    entt::entity EntityFactory::CreateEmptyEntity(entt::registry& registry, const std::string& name, const Math::Vec3& position)
    {
        entt::entity entity = registry.create();

        // Toda entidade recebe TagComponent e Transform obrigatoriamente
        registry.emplace<TagComponent>(entity, TagComponent{ name });

        Math::Transform transform;
        transform.translation = position;
        registry.emplace<Math::Transform>(entity, transform);

        return entity;
    }

    entt::entity EntityFactory::CreateStaticMeshEntity(entt::registry& registry, AssetManager* assetManager, const std::string& name, const Math::Vec3& position)
    {
        entt::entity entity = CreateEmptyEntity(registry, name, position);

        MeshComponent meshComp{};
        if (assetManager) {
            meshComp.meshAsset = assetManager->Load<Mesh>("box.obj");
        }
        registry.emplace<MeshComponent>(entity, meshComp);

        return entity;
    }

    entt::entity EntityFactory::CreateDirectionalLightEntity(entt::registry& registry, const std::string& name)
    {
        entt::entity entity = CreateEmptyEntity(registry, name, Math::Vec3(0.0f, 10.0f, 0.0f));

        DirectionalLightComponent lightComp{};
        lightComp.color = Math::Vec3(1.0f, 0.95f, 0.8f);
        lightComp.intensity = 1.0f;
        lightComp.direction = Math::Vec3(-0.5f, -1.0f, -0.5f);

        registry.emplace<DirectionalLightComponent>(entity, lightComp);

        return entity;
    }

    entt::entity EntityFactory::CreatePointLightEntity(entt::registry& registry, const std::string& name, const Math::Vec3& position)
    {
        entt::entity entity = CreateEmptyEntity(registry, name, position);

        PointLightComponent lightComp{};
        lightComp.color = Math::Vec3(1.0f, 1.0f, 1.0f);
        lightComp.intensity = 5.0f;
        lightComp.radius = 10.0f;

        registry.emplace<PointLightComponent>(entity, lightComp);

        return entity;
    }

    entt::entity EntityFactory::CreateCameraEntity(entt::registry& registry, const std::string& name, const Math::Vec3& position)
    {
        entt::entity entity = CreateEmptyEntity(registry, name, position);

        CameraComponent camComp{};
        camComp.isPrimary = false;
        camComp.fov = 60.0f;
        camComp.nearClip = 0.1f;
        camComp.farClip = 1000.0f;

        registry.emplace<CameraComponent>(entity, camComp);

        return entity;
    }

} // namespace Dan