#include <Dan/Common/Reflection/ReflectionRegistry.hpp>
#include <Dan/Common/Math/MathTypes.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <Dan/Common/Components/PhysicsComponents.hpp>
#include <Dan/Common/Components/TagComponent.hpp>
#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Dan::Reflection {

	using namespace entt::literals;

	// =========================================================================
	// Helpers de Acoplamento Dinâmico (Necessário para o SceneSerializer genérico)
	// =========================================================================
	template<typename Component>
	static void AssignComponent(
		entt::registry& registry,
		entt::entity entity,
		const Component& instance)
	{
		registry.emplace_or_replace<Component>(entity, instance);
	}

	template<typename Component>
	static entt::meta_any GetComponent(entt::registry& registry, entt::entity entity)
	{
		if (auto* comp = registry.try_get<Component>(entity)) {
			// forward_as_meta cria um ponteiro opaco sem copiar o objeto em memória
			return entt::forward_as_meta(*comp);
		}
		return entt::meta_any{};
	}

	// Macro auxiliar para reduzir o boilerplate no registro de componentes de Entidade
#define REGISTER_COMPONENT(Type, Name) \
        entt::meta_factory<Type>() \
            .type(Name##_hs, Name) \
            .func<&AssignComponent<Type>>("assign"_hs) \
            .func<&GetComponent<Type>>("get"_hs)

	// Encadeia a reflexão de um membro de dados (variável/campo da struct)
#define REGISTER_FIELD(Class, Field, Name) \
        .data<&Class::Field>(entt::hashed_string{Name}, Name)

	// =========================================================================
	// Registro Principal
	// =========================================================================
	void RegisterEngineTypes()
	{

		// 1. Tipos Básicos e Matemáticos[cite: 3]
		entt::meta_factory<Math::Vec3>()
			.type("Vec3"_hs, "Vec3")
			.data<&Math::Vec3::x>("X"_hs)
			.data<&Math::Vec3::y>("Y"_hs)
			.data<&Math::Vec3::z>("Z"_hs);

		entt::meta_factory<Math::Vec4>()
			.type("Vec4"_hs, "Vec4")
			.data<&Math::Vec4::x>("X"_hs)
			.data<&Math::Vec4::y>("Y"_hs)
			.data<&Math::Vec4::z>("Z"_hs)
			.data<&Math::Vec4::w>("W"_hs);

		// GLM Quat (Rotina interna opaca, o SceneSerializer o converte para Euler no JSON)[cite: 8]
		entt::meta_factory<glm::quat>()
			.type("Quat"_hs, "Quat")
			.data<&glm::quat::x>("X"_hs)
			.data<&glm::quat::y>("Y"_hs)
			.data<&glm::quat::z>("Z"_hs)
			.data<&glm::quat::w>("W"_hs);

		// 2. Transform
		REGISTER_COMPONENT(Math::Transform, "Transform")
			.data<&Math::Transform::translation>("Translation"_hs, "Translation")
			.data<&Math::Transform::rotation>("Rotation"_hs, "Rotation")
			.data<&Math::Transform::scale>("Scale"_hs, "Scale");

		// 3. Câmera
		REGISTER_COMPONENT(CameraComponent, "CameraComponent")
			.data<&CameraComponent::isPrimary>("IsPrimary"_hs, "IsPrimary")
			.data<&CameraComponent::fov>("Fov"_hs, "Fov")
			.data<&CameraComponent::nearClip>("NearClip"_hs, "NearClip")
			.data<&CameraComponent::farClip>("FarClip"_hs, "FarClip")
			.data<&CameraComponent::yaw>("Yaw"_hs, "Yaw")
			.data<&CameraComponent::pitch>("Pitch"_hs, "Pitch")
			.data<&CameraComponent::mouseSensitivity>("MouseSensitivity"_hs, "MouseSensitivity")
			.data<&CameraComponent::baseSpeed>("BaseSpeed"_hs, "BaseSpeed");

		// 4. Luzes
		REGISTER_COMPONENT(DirectionalLightComponent, "DirectionalLightComponent")
			.data<&DirectionalLightComponent::direction>("Direction"_hs, "Direction")
			.data<&DirectionalLightComponent::color>("Color"_hs, "Color")
			.data<&DirectionalLightComponent::intensity>("Intensity"_hs, "Intensity");

		REGISTER_COMPONENT(PointLightComponent, "PointLightComponent")
			.data<&PointLightComponent::color>("Color"_hs, "Color")
			.data<&PointLightComponent::intensity>("Intensity"_hs, "Intensity")
			.data<&PointLightComponent::radius>("Radius"_hs, "Radius");

		// 5. Física (Jolt)
		entt::meta_factory<MotionType>().type("MotionType"_hs, "MotionType");
		 
		// Se as enumerations precisarem ser refletidas, devem ser declaradas separadamente,
		// mas aqui vamos tratar propriedades base para refletir.
		REGISTER_COMPONENT(RigidBodyComponent, "RigidBodyComponent")
			.data<&RigidBodyComponent::motionType>("MotionType"_hs, "MotionType")
			.data<&RigidBodyComponent::mass>("Mass"_hs, "Mass")
			.data<&RigidBodyComponent::allowSleeping>("AllowSleeping"_hs, "AllowSleeping");

		REGISTER_COMPONENT(TagComponent, "TagComponent")
			REGISTER_FIELD(TagComponent, tag, "Tag");

		// Nota sobre MeshComponent e ColliderComponent:
		// Por conterem tipos dinâmicos (std::shared_ptr, std::variant), não é estritamente 
		// necessário injetar todos os seus membros nativos na interface do entt::meta de forma 
		// rasa. Caso o Editor Qt precise visualizá-los diretamente no futuro, você precisará 
		// registrar as classes "Material" e "Mesh" ou tratá-los de forma especializada no Serializer[cite: 11].

	}

} // namespace Dan::Reflection