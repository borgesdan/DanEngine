#include <Dan/Common/Scene/SceneSerializer.hpp>
#include <Dan/Common/Assets/AssetManager.hpp>
#include <Dan/Common/IO/FileSystem.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include  <Dan/Common/Assets/TerrainBuilder.hpp>

// Componentes do ECS
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <Dan/Common/Components/PhysicsComponents.hpp>
#include <Dan/Common/Components/TerrainComponent.hpp>

#include <nlohmann/json.hpp>
#include <fstream>
#include <glm/gtc/quaternion.hpp>

using json = nlohmann::json;

namespace Dan {

	// Conversões para GLM Vec3 e Quat
	static json SerializeVec3(const Math::Vec3& vec) {
		return json::array({ vec.x, vec.y, vec.z });
	}

	static Math::Vec3 DeserializeVec3(const json& j) {
		return Math::Vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
	}

	static json SerializeQuat(const glm::quat& q) {
		Math::Vec3 eulerDegrees = glm::degrees(glm::eulerAngles(q));
		return SerializeVec3(eulerDegrees);
	}

	static glm::quat DeserializeQuat(const json& j) {
		Math::Vec3 eulerDegrees = DeserializeVec3(j);
		return glm::quat(glm::radians(eulerDegrees));
	}

	// Converters para MotionType
	static std::string MotionTypeToString(MotionType motionType) {
		switch (motionType) {
		case MotionType::Static:    return "Static";
		case MotionType::Kinematic: return "Kinematic";
		case MotionType::Dynamic:
		default:                    return "Dynamic";
		}
	}

	static MotionType StringToMotionType(const std::string& str) {
		if (str == "Static")    return MotionType::Static;
		if (str == "Kinematic") return MotionType::Kinematic;
		return MotionType::Dynamic;
	}

	// Funções auxiliares para carregar os dados JSON

	static void LoadDirectionalLightComponent(json const& entityData, entt::registry& registry, entt::entity entity)
	{
		if (entityData.contains("DirectionalLightComponent"))
		{
			const auto& lightData = entityData["DirectionalLightComponent"];
			DirectionalLightComponent light;

			if (lightData.contains("Direction")) {
				light.direction = DeserializeVec3(lightData["Direction"]);
			}
			if (lightData.contains("Color")) {
				light.color = DeserializeVec3(lightData["Color"]);
			}
			light.intensity = lightData.value("Intensity", 1.0f);

			registry.emplace<DirectionalLightComponent>(entity, light);
		}
	}

	static void LoadTransform(json const& entityData, entt::registry& registry, entt::entity entity) {
		if (entityData.contains("Transform"))
		{
			const auto& transformData = entityData["Transform"];
			Math::Transform transform;

			transform.translation = { transformData["Translation"][0], transformData["Translation"][1], transformData["Translation"][2] };

			// Conversão de Euler para Quaternion
			Math::Vec3 eulerRotation = { transformData["Rotation"][0], transformData["Rotation"][1], transformData["Rotation"][2] };
			transform.rotation = glm::quat(glm::radians(eulerRotation));

			transform.scale = { transformData["Scale"][0], transformData["Scale"][1], transformData["Scale"][2] };

			registry.emplace<Math::Transform>(entity, transform);
		}

	}

	static void LoadMeshComponent(json const& entityData, entt::registry& registry, entt::entity entity, AssetManager* assetManager)
	{
		if (entityData.contains("MeshComponent"))
		{
			const auto& meshData = entityData["MeshComponent"];
			auto mesh = assetManager->Load<Mesh>(meshData["MeshPath"]);
			auto vs = assetManager->Load<Shader>(meshData["VertexShader"]);
			auto fs = assetManager->Load<Shader>(meshData["FragmentShader"]);
			if (mesh && vs && fs) {
				auto material = std::make_shared<Material>(vs, fs);
				registry.emplace<MeshComponent>(entity, MeshComponent{ mesh, material });
			}
			else {
				DAN_ENGINE_WARN("Falha ao carregar dependências visuais da entidade.");
			}
		}
	}

	static void LoadCameraComponent(json const& entityData, entt::registry& registry, entt::entity entity)
	{
		if (entityData.contains("CameraComponent"))
		{
			const auto& camData = entityData["CameraComponent"];
			CameraComponent camera;
			camera.isPrimary = camData["IsPrimary"];
			camera.fov = camData["Fov"];
			camera.nearClip = camData["NearClip"];
			camera.farClip = camData["FarClip"];
			registry.emplace<CameraComponent>(entity, camera);
		}
	}

	static void LoadRigidBodyComponent(json const& entityData, entt::registry& registry, entt::entity entity)
	{
		if (entityData.contains("RigidBodyComponent"))
		{
			const auto& rbData = entityData["RigidBodyComponent"];
			RigidBodyComponent rb;
			rb.motionType = StringToMotionType(rbData.value("MotionType", "Dynamic"));
			rb.mass = rbData.value("Mass", 1.0f);
			rb.allowSleeping = rbData.value("AllowSleeping", true);
			registry.emplace<RigidBodyComponent>(entity, rb);
		}
	}

	static void LoadColliderComponent(json const& entityData, entt::registry& registry, entt::entity entity)
	{
		if (entityData.contains("ColliderComponent"))
		{
			const auto& colData = entityData["ColliderComponent"];
			ColliderComponent collider;
			collider.isTrigger = colData.value("IsTrigger", false);
			collider.friction = colData.value("Friction", 0.5f);
			collider.restitution = colData.value("Restitution", 0.1f);
			if (colData.contains("CenterOffset")) {
				collider.centerOffset = DeserializeVec3(colData["CenterOffset"]);
			}
			std::string shapeType = colData.value("Shape", "Box");
			if (shapeType == "Box") {
				BoxColliderShape boxShape;
				if (colData.contains("HalfExtents")) {
					boxShape.halfExtents = DeserializeVec3(colData["HalfExtents"]);
				}
				collider.shape = boxShape;
			}
			else if (shapeType == "Sphere") {
				SphereColliderShape sphereShape;
				sphereShape.radius = colData.value("Radius", 0.5f);
				collider.shape = sphereShape;
			}
			else if (shapeType == "Capsule") {
				CapsuleColliderShape capsuleShape;
				capsuleShape.halfHeight = colData.value("HalfHeight", 0.5f);
				capsuleShape.radius = colData.value("Radius", 0.25f);
				collider.shape = capsuleShape;
			}
			else if (shapeType == "Mesh") {
				MeshColliderShape meshShape;
				meshShape.isConvex = colData.value("IsConvex", true);
				collider.shape = meshShape;
			}
			registry.emplace<ColliderComponent>(entity, collider);
		}
	}

	static void LoadTerrainComponent(json const& entityData, entt::registry& registry, entt::entity entity, AssetManager* assetManager, FileSystem* fileSystem)
	{
		if (entityData.contains("TerrainComponent"))
		{
			auto terrainData = entityData["TerrainComponent"];
			TerrainComponent terrain{};
			terrain.width = terrainData.value("Width", 257);
			terrain.height = terrainData.value("Height", 257);
			terrain.heightScale = terrainData.value("HeightScale", 50.0f);
			terrain.heightmapPath = terrainData.value("HeightmapPath", "");
			if (terrainData.contains("GridSpacing"))
			{
				auto spacing = terrainData["GridSpacing"];
				terrain.gridSpacing = { spacing[0], spacing[1], spacing[2] };
			}
			// Carregamento dos dados brutos do Heightmap via FileSystem
			if (!terrain.heightmapPath.empty())
			{
				std::vector<uint8_t> rawBytes = fileSystem->ReadBinaryFile(terrain.heightmapPath);
				if (!rawBytes.empty())
				{
					terrain.heightData.resize(terrain.width * terrain.height);
					// Converte os dados de bytes/16-bit para float normalizado [0.0, 1.0]
					const uint16_t* raw16 = reinterpret_cast<const uint16_t*>(rawBytes.data());
					for (size_t i = 0; i < terrain.heightData.size(); ++i)
					{
						terrain.heightData[i] = static_cast<float>(raw16[i]) / 65535.0f;
					}
				}
			}
			// Instancia a malha e adiciona o MeshComponent para renderização
			auto terrainMesh = TerrainBuilder::BuildMesh(terrain);
			auto vsTerrain = assetManager->Load<Shader>("Shaders/vs_mesh.bin");
			auto fsTerrain = assetManager->Load<Shader>("Shaders/fs_mesh.bin");
			if (terrainMesh && vsTerrain && fsTerrain)
			{
				auto material = std::make_shared<Material>(vsTerrain, fsTerrain);
				registry.emplace<MeshComponent>(entity, MeshComponent{ terrainMesh, material });
			}
			registry.emplace<TerrainComponent>(entity, std::move(terrain));
		}
	}

	// Funções auxiliares para salvar os dados JSON
	static void SaveTransform(entt::registry& registry, entt::entity entity, json& entityJson) {
		if (auto* transform = registry.try_get<Math::Transform>(entity)) {
			json transformJson;
			transformJson["Translation"] = SerializeVec3(transform->translation);
			transformJson["Rotation"] = SerializeQuat(transform->rotation);
			transformJson["Scale"] = SerializeVec3(transform->scale);
			entityJson["Transform"] = transformJson;
		}
	}

	static void SaveCameraComponent(entt::registry& registry, entt::entity entity, json& entityJson) {
		if (auto* camera = registry.try_get<CameraComponent>(entity)) {
			json camJson;
			camJson["IsPrimary"] = camera->isPrimary;
			camJson["Fov"] = camera->fov;
			camJson["NearClip"] = camera->nearClip;
			camJson["FarClip"] = camera->farClip;
			entityJson["CameraComponent"] = camJson;
		}
	}

	static void SaveRigidBodyComponent(entt::registry& registry, entt::entity entity, json& entityJson) {
		if (auto* rb = registry.try_get<RigidBodyComponent>(entity)) {
			json rbJson;
			rbJson["MotionType"] = MotionTypeToString(rb->motionType);
			rbJson["Mass"] = rb->mass;
			rbJson["AllowSleeping"] = rb->allowSleeping;
			entityJson["RigidBodyComponent"] = rbJson;
		}
	}

	static void SaveColliderComponent(entt::registry& registry, entt::entity entity, json& entityJson) {
		if (auto* collider = registry.try_get<ColliderComponent>(entity)) {
			json colJson;
			colJson["IsTrigger"] = collider->isTrigger;
			colJson["Friction"] = collider->friction;
			colJson["Restitution"] = collider->restitution;
			colJson["CenterOffset"] = SerializeVec3(collider->centerOffset);
			std::visit([&colJson](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, BoxColliderShape>) {
					colJson["Shape"] = "Box";
					colJson["HalfExtents"] = SerializeVec3(arg.halfExtents);
				}
				else if constexpr (std::is_same_v<T, SphereColliderShape>) {
					colJson["Shape"] = "Sphere";
					colJson["Radius"] = arg.radius;
				}
				else if constexpr (std::is_same_v<T, CapsuleColliderShape>) {
					colJson["Shape"] = "Capsule";
					colJson["HalfHeight"] = arg.halfHeight;
					colJson["Radius"] = arg.radius;
				}
				else if constexpr (std::is_same_v<T, MeshColliderShape>) {
					colJson["Shape"] = "Mesh";
					colJson["IsConvex"] = arg.isConvex;
				}
				}, collider->shape);
			entityJson["ColliderComponent"] = colJson;
		}
	}

	static void SaveDirectionalLightComponent(entt::registry& registry, entt::entity entity, json& entityJson) {
		if (auto* light = registry.try_get<DirectionalLightComponent>(entity))
		{
			json lightJson;
			lightJson["Direction"] = SerializeVec3(light->direction);
			lightJson["Color"] = SerializeVec3(light->color);
			lightJson["Intensity"] = light->intensity;
			entityJson["DirectionalLightComponent"] = lightJson;
		}
	}

	static void SaveMeshComponent(entt::registry& registry, entt::entity entity, json& entityJson)
	{
		if (auto* meshComp = registry.try_get<MeshComponent>(entity))
		{
			json meshJson;

			if (meshComp->meshAsset) {
				meshJson["MeshPath"] = meshComp->meshAsset->GetVirtualPath();
			}

			if (meshComp->material) {
				json matJson;
				matJson["VertexShader"] = meshComp->material->GetVertexShader()->GetVirtualPath();
				matJson["FragmentShader"] = meshComp->material->GetFragmentShader()->GetVirtualPath();
				/*matJson["AlbedoTexture"] = meshComp->material->GetAlbedoPath();
				matJson["NormalMap"] = meshComp->material->GetNormalMapPath();
				matJson["SpecularMap"] = meshComp->material->GetSpecularMapPath();*/

				Math::Vec4 props = meshComp->material->GetPackedProperties();
				matJson["BaseColor"] = SerializeVec3(Math::Vec3(props.x, props.y, props.z));
				matJson["Roughness"] = props.w;

				meshJson["Material"] = matJson;
			}

			entityJson["MeshComponent"] = meshJson;
		}
	}

	static void SaveTerrainComponent(entt::registry& registry, entt::entity entity, json& entityJson)
	{
		if (auto* terrain = registry.try_get<TerrainComponent>(entity))
		{
			json terrainJson;
			terrainJson["Width"] = terrain->width;
			terrainJson["Height"] = terrain->height;
			terrainJson["HeightScale"] = terrain->heightScale;
			terrainJson["GridSpacing"] = SerializeVec3(terrain->gridSpacing);
			terrainJson["HeightmapPath"] = terrain->heightmapPath;

			entityJson["TerrainComponent"] = terrainJson;
		}
	}


	SceneSerializer::SceneSerializer(entt::registry& registry, AssetManager* assetManager, FileSystem* fileSystem)
		: m_registry(registry), m_assetManager(assetManager), m_fileSystem(fileSystem) {}

	bool SceneSerializer::Load(const std::string& filepath)
	{
		std::string jsonString = m_fileSystem->ReadTextFile(filepath);
		if (jsonString.empty()) return false;

		json data;
		try {
			data = json::parse(jsonString);
		}
		catch (json::parse_error& e) {
			DAN_ENGINE_ERROR("Falha no parse do arquivo de cena {0}: {1}", filepath, e.what());
			return false;
		}

		DAN_ENGINE_INFO("Carregando cena: {0}", data["Scene"].get<std::string>());

		const auto& entities = data["Entities"];
		if (entities.is_array())
		{
			for (const auto& entityData : entities)
			{
				auto entity = m_registry.create();

				LoadTransform(entityData, m_registry, entity);
				LoadMeshComponent(entityData, m_registry, entity, m_assetManager);
				LoadCameraComponent(entityData, m_registry, entity);
				LoadRigidBodyComponent(entityData, m_registry, entity);
				LoadColliderComponent(entityData, m_registry, entity);
				LoadTerrainComponent(entityData, m_registry, entity, m_assetManager, m_fileSystem);
				LoadDirectionalLightComponent(entityData, m_registry, entity);
			}
		}
		return true;
	}

	bool SceneSerializer::Save(const std::string& filepath)
	{
		DAN_CORE_ASSERT(m_fileSystem != nullptr, "FileSystem invalido no SceneSerializer!");

		json sceneJson;
		sceneJson["Scene"] = "MainLevel";
		json entitiesArray = json::array();

		// 1. Obtém uma View de todas as entidades vivas registradas no EnTT
		auto allEntities = m_registry.view<entt::entity>();

		for (auto entityID : allEntities) {
			json entityJson;
			entityJson["ID"] = static_cast<uint32_t>(entityID);
			
			SaveTransform(m_registry, entityID, entityJson);
			SaveCameraComponent(m_registry, entityID, entityJson);
			SaveRigidBodyComponent(m_registry, entityID, entityJson);
			SaveColliderComponent(m_registry, entityID, entityJson);
			SaveDirectionalLightComponent(m_registry, entityID, entityJson);
			SaveMeshComponent(m_registry, entityID, entityJson);
			SaveTerrainComponent(m_registry, entityID, entityJson);

			// Insere a entidade serializada no array final
			entitiesArray.push_back(entityJson);
		};

		sceneJson["Entities"] = entitiesArray;

		// 2. Gravação em disco através do caminho raiz do FileSystem
		std::filesystem::path fullPath = m_fileSystem->GetRootPath() / filepath;

		// Garante que o diretório de destino exista
		if (fullPath.has_parent_path()) {
			std::filesystem::create_directories(fullPath.parent_path());
		}

		std::ofstream outFile(fullPath);
		if (!outFile.is_open()) {
			DAN_ENGINE_ERROR("SceneSerializer: Falha ao abrir o arquivo para escrita em {0}", fullPath.string());
			return false;
		}

		// Escreve a árvore JSON formatada com recuo de 4 espaços
		outFile << sceneJson.dump(4);
		outFile.close();

		DAN_ENGINE_INFO("Cena salva com sucesso em: {0}", filepath);
		return true;
	}

} // namespace Dan