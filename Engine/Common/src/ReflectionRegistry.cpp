#include <Dan/Common/Reflection/ReflectionRegistry.hpp>
#include <Dan/Common/Math/MathTypes.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Components/TagComponent.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <Dan/Common/Components/PhysicsComponents.hpp>

#include <entt/entt.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

namespace Dan::Reflection {

    using namespace entt::literals;

    // Helpers de Lifecycle de Componentes no EnTT
    template<typename Component>
    static void AssignComponent(entt::registry& registry, entt::entity entity, const Component& instance) {
        registry.emplace_or_replace<Component>(entity, instance);
    }

    template<typename Component>
    static entt::meta_any GetComponent(entt::registry& registry, entt::entity entity) {
        if (auto* comp = registry.try_get<Component>(entity)) {
            return entt::forward_as_meta(*comp);
        }
        return entt::meta_any{};
    }

    template<typename Component>
    static void EmplaceComponent(entt::registry& registry, entt::entity entity) {
        registry.emplace_or_replace<Component>(entity);
    }

    template<typename Component>
    static void RemoveComponent(entt::registry& registry, entt::entity entity) {
        registry.remove<Component>(entity);
    }

    // Macro atualizada com funções de atribuição, leitura, alocação e remoção
#define REGISTER_COMPONENT(Type, Name) \
        entt::meta_factory<Type>() \
            .type(entt::hashed_string(Name), Name) \
            .func<&AssignComponent<Type>>("assign"_hs) \
            .func<&GetComponent<Type>>("get"_hs) \
            .func<&EmplaceComponent<Type>>("emplace"_hs) \
            .func<&RemoveComponent<Type>>("remove"_hs)

#define REGISTER_FIELD(Class, Field, Name) \
        .data<&Class::Field>(entt::hashed_string(Name), Name)

    void RegisterEngineTypes()
    {
        // 1. Tipos Matemáticos e Primitivos
        entt::meta_factory<std::string>().type("String"_hs, "String");
        entt::meta_factory<Math::Vec3>().type("Vec3"_hs, "Vec3");
        entt::meta_factory<Math::Vec4>().type("Vec4"_hs, "Vec4");
        entt::meta_factory<glm::quat>().type("Quat"_hs, "Quat");
        entt::meta_factory<MotionType>().type("MotionType"_hs, "MotionType");

        // 2. TagComponent
        REGISTER_COMPONENT(TagComponent, "TagComponent")
            REGISTER_FIELD(TagComponent, tag, "Tag")
            REGISTER_FIELD(TagComponent, layer, "Layer");

        // 3. Transform
        REGISTER_COMPONENT(Math::Transform, "Transform")
            REGISTER_FIELD(Math::Transform, translation, "Translation")
            REGISTER_FIELD(Math::Transform, rotation, "Rotation")
            REGISTER_FIELD(Math::Transform, scale, "Scale");

        // 4. CameraComponent
        REGISTER_COMPONENT(CameraComponent, "CameraComponent")
            REGISTER_FIELD(CameraComponent, isPrimary, "IsPrimary")
            REGISTER_FIELD(CameraComponent, fov, "Fov")
            REGISTER_FIELD(CameraComponent, nearClip, "NearClip")
            REGISTER_FIELD(CameraComponent, farClip, "FarClip")
            REGISTER_FIELD(CameraComponent, yaw, "Yaw")
            REGISTER_FIELD(CameraComponent, pitch, "Pitch")
            REGISTER_FIELD(CameraComponent, mouseSensitivity, "MouseSensitivity")
            REGISTER_FIELD(CameraComponent, baseSpeed, "BaseSpeed");

        // 5. LightComponents
        REGISTER_COMPONENT(DirectionalLightComponent, "DirectionalLightComponent")
            REGISTER_FIELD(DirectionalLightComponent, direction, "Direction")
            REGISTER_FIELD(DirectionalLightComponent, color, "Color")
            REGISTER_FIELD(DirectionalLightComponent, intensity, "Intensity");

        REGISTER_COMPONENT(PointLightComponent, "PointLightComponent")
            REGISTER_FIELD(PointLightComponent, color, "Color")
            REGISTER_FIELD(PointLightComponent, intensity, "Intensity")
            REGISTER_FIELD(PointLightComponent, radius, "Radius");

        // 6. RigidBodyComponent
        REGISTER_COMPONENT(RigidBodyComponent, "RigidBodyComponent")
            REGISTER_FIELD(RigidBodyComponent, motionType, "MotionType")
            REGISTER_FIELD(RigidBodyComponent, mass, "Mass")
            REGISTER_FIELD(RigidBodyComponent, allowSleeping, "AllowSleeping");
    }

} // namespace Dan::Reflection