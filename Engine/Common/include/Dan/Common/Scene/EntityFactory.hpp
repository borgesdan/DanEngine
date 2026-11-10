#pragma once
#include <entt/entt.hpp>
#include <string>
#include <Dan/Common/Math/Transform.hpp>

namespace Dan {

    class AssetManager;

    class EntityFactory {
    public:
        static entt::entity CreateEmptyEntity(entt::registry& registry, const std::string& name = "Empty Entity", const Math::Vec3& position = Math::Vec3(0.0f));
        static entt::entity CreateStaticMeshEntity(entt::registry& registry, AssetManager* assetManager, const std::string& name = "Static Mesh", const Math::Vec3& position = Math::Vec3(0.0f));
        static entt::entity CreateDirectionalLightEntity(entt::registry& registry, const std::string& name = "Directional Light");
        static entt::entity CreatePointLightEntity(entt::registry& registry, const std::string& name = "Point Light", const Math::Vec3& position = Math::Vec3(0.0f));
        static entt::entity CreateCameraEntity(entt::registry& registry, const std::string& name = "Camera", const Math::Vec3& position = Math::Vec3(0.0f));
    };

} // namespace Dan