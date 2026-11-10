#include <Dan/Common/Scene/SceneSerializer.hpp>
#include <Dan/Common/Assets/AssetManager.hpp>
#include <Dan/Common/IO/FileSystem.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <Dan/Common/Components/PhysicsComponents.hpp>
#include <Dan/Common/Components/TerrainComponent.hpp>
#include <Dan/Common/Assets/TerrainBuilder.hpp>

#include <nlohmann/json.hpp>
#include <fstream>
#include <entt/entt.hpp>
#include <glm/gtc/quaternion.hpp>

using json = nlohmann::json;
using namespace entt::literals;

namespace Dan {

	static json SerializeVec3(const Math::Vec3& vec) {
		return json::array({ vec.x, vec.y, vec.z });
	}

	static Math::Vec3 DeserializeVec3(const json& j) {
		return Math::Vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
	}

	// =========================================================================
	// Funções Data-Driven via entt::meta
	// =========================================================================
	static void SerializeMetaType(const entt::any& instance, json& outJson)
	{
		entt::meta_type metaType = entt::resolve(instance.type());

		for (auto [id, metaData] : metaType.data())
		{
			std::string propName(metaData.name());
			if (propName.empty())
				continue;

			entt::meta_any propValue = metaData.get(instance);

			if (!propValue)
				continue;

			// Tipos numéricos
			if (propValue.allow_cast<float>())
			{
				outJson[propName] = propValue.cast<float>();
			}
			// Boolean
			else if (propValue.allow_cast<bool>())
			{
				outJson[propName] = propValue.cast<bool>();
			}
			// Vec3
			else if (propValue.allow_cast<Math::Vec3>())
			{
				Math::Vec3 v = propValue.cast<Math::Vec3>();

				outJson[propName] = json::array({
					v.x, v.y, v.z
					});
			}
			// Quaternion
			else if (propValue.allow_cast<glm::quat>())
			{
				glm::quat q = propValue.cast<glm::quat>();

				Math::Vec3 euler = glm::degrees(glm::eulerAngles(q));

				outJson[propName] = json::array({
					euler.x, euler.y, euler.z
					});
			}
		}
	}

	// =========================================================================
	// Funções Data-Driven via entt::meta
	// =========================================================================
	static void DeserializeMetaType(const json& inJson, entt::meta_any& instance)
	{
		entt::meta_type metaType = instance.type();

		for (auto [id, metaData] : metaType.data())
		{
			if (metaData.name() == nullptr)
			{
				DAN_ENGINE_WARN("Propriedade refletida sem nome encontrada no tipo {0}, ignorando...", metaType.name());
				continue;
			}

			std::string propName(metaData.name());

			if (propName.empty() || !inJson.contains(propName))
				continue;

			const auto& jValue = inJson[propName];

			entt::meta_any propValue = metaData.get(instance);

			if (!propValue)
				continue;

			if (propValue.type() == entt::resolve<float>())
			{
				if (jValue.is_number())
					metaData.set(instance, jValue.get<float>());
			}
			else if (propValue.type() == entt::resolve<bool>())
			{
				if (jValue.is_boolean())
					metaData.set(instance, jValue.get<bool>());
			}
			else if (propValue.type() == entt::resolve<Math::Vec3>())
			{
				if (jValue.is_array() && jValue.size() == 3)
				{
					Math::Vec3 v(
						jValue[0].get<float>(),
						jValue[1].get<float>(),
						jValue[2].get<float>()
					);

					metaData.set(instance, v);
				}
			}
			else if (propValue.type() == entt::resolve<glm::quat>())
			{
				if (jValue.is_array() && jValue.size() == 3)
				{
					Math::Vec3 euler(
						jValue[0].get<float>(),
						jValue[1].get<float>(),
						jValue[2].get<float>()
					);

					glm::quat q(glm::radians(euler));

					metaData.set(instance, q);
				}
			}
		}
	}

	// =========================================================================
	// Fallback para Componentes com Alocações Complexas (Mesh, Terrain, Collider)
	// =========================================================================
	static void SaveComplexComponents(entt::registry& registry, entt::entity entityID, json& entityJson)
	{
		// 1. MeshComponent (Armazena os caminhos virtuais da malha e dos shaders do Material)[cite: 3]
		if (auto* meshComp = registry.try_get<MeshComponent>(entityID))
		{
			json meshJson;
			if (meshComp->meshAsset) {
				meshJson["MeshPath"] = meshComp->meshAsset->GetVirtualPath();
			}
			if (meshComp->material) {
				json matJson;
				if (meshComp->material->GetVertexShader())
					matJson["VertexShader"] = meshComp->material->GetVertexShader()->GetVirtualPath();
				if (meshComp->material->GetFragmentShader())
					matJson["FragmentShader"] = meshComp->material->GetFragmentShader()->GetVirtualPath();

				Math::Vec4 props = meshComp->material->GetPackedProperties();
				matJson["BaseColor"] = SerializeVec3(Math::Vec3(props.x, props.y, props.z));
				matJson["Roughness"] = props.w;
				meshJson["Material"] = matJson;
			}
			entityJson["MeshComponent"] = meshJson;
		}

		// 2. TerrainComponent (Guarda as dimensões e o caminho do arquivo de elevação .raw)[cite: 2]
		if (auto* terrain = registry.try_get<TerrainComponent>(entityID))
		{
			json terrainJson;
			terrainJson["Width"] = terrain->width;
			terrainJson["Height"] = terrain->height;
			terrainJson["HeightScale"] = terrain->heightScale;
			terrainJson["GridSpacing"] = SerializeVec3(terrain->gridSpacing);
			terrainJson["HeightmapPath"] = terrain->heightmapPath;
			entityJson["TerrainComponent"] = terrainJson;
		}

		// 3. ColliderComponent (Usa std::visit para extrair os dados do std::variant)[cite: 3]
		if (auto* collider = registry.try_get<ColliderComponent>(entityID))
		{
			json colJson;
			colJson["CenterOffset"] = SerializeVec3(collider->centerOffset);
			colJson["IsTrigger"] = collider->isTrigger;
			colJson["Friction"] = collider->friction;
			colJson["Restitution"] = collider->restitution;

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

	static void LoadComplexComponents(const json& entityData, entt::registry& registry, entt::entity entity, AssetManager* assetManager, FileSystem* fileSystem)
	{
		// 1. MeshComponent (Carrega malha e instancia o Material via AssetManager)[cite: 3]
		if (entityData.contains("MeshComponent"))
		{
			const auto& meshData = entityData["MeshComponent"];
			std::string meshPath = meshData.value("MeshPath", "");
			auto mesh = !meshPath.empty() ? assetManager->Load<Mesh>(meshPath) : nullptr;

			if (meshData.contains("Material")) {
				const auto& matData = meshData["Material"];
				auto vs = assetManager->Load<Shader>(matData.value("VertexShader", "Shaders/vs_mesh.bin"));
				auto fs = assetManager->Load<Shader>(matData.value("FragmentShader", "Shaders/fs_mesh.bin"));

				if (mesh && vs && fs) {
					auto material = std::make_shared<Material>(vs, fs);
					registry.emplace<MeshComponent>(entity, MeshComponent{ mesh, material });
				}
			}
		}

		// 2. TerrainComponent (Lê o arquivo binário .raw, constrói os dados de altura e gera a malha via TerrainBuilder)[cite: 2]
		if (entityData.contains("TerrainComponent"))
		{
			const auto& terrainData = entityData["TerrainComponent"];
			TerrainComponent terrain{};

			terrain.width = terrainData.value("Width", 257);
			terrain.height = terrainData.value("Height", 257);
			terrain.heightScale = terrainData.value("HeightScale", 40.0f);
			terrain.heightmapPath = terrainData.value("HeightmapPath", "");

			if (terrainData.contains("GridSpacing")) {
				terrain.gridSpacing = DeserializeVec3(terrainData["GridSpacing"]);
			}

			// Carrega o buffer de elevação do disco se o caminho existir
			if (!terrain.heightmapPath.empty() && fileSystem) {
				std::vector<uint8_t> rawBytes = fileSystem->ReadBinaryFile(terrain.heightmapPath);
				if (!rawBytes.empty()) {
					terrain.heightData.resize(terrain.width * terrain.height);
					const uint16_t* raw16 = reinterpret_cast<const uint16_t*>(rawBytes.data());
					for (size_t i = 0; i < terrain.heightData.size(); ++i) {
						terrain.heightData[i] = static_cast<float>(raw16[i]) / 65535.0f;
					}
				}
			}

			// Gera a malha procedural no GPU via bgfx[cite: 2]
			auto terrainMesh = TerrainBuilder::BuildMesh(terrain);
			auto vsTerrain = assetManager->Load<Shader>("Shaders/vs_mesh.bin");
			auto fsTerrain = assetManager->Load<Shader>("Shaders/fs_mesh.bin");

			if (terrainMesh && vsTerrain && fsTerrain) {
				auto material = std::make_shared<Material>(vsTerrain, fsTerrain);
				registry.emplace<MeshComponent>(entity, MeshComponent{ terrainMesh, material });
			}

			registry.emplace<TerrainComponent>(entity, std::move(terrain));
		}

		// 3. ColliderComponent (Desserializa a forma geométrica específica no std::variant)[cite: 3]
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

	// =========================================================================
	// Métodos Principais do SceneSerializer
	// =========================================================================
	SceneSerializer::SceneSerializer(entt::registry& registry, AssetManager* assetManager, FileSystem* fileSystem)
		: m_registry(registry), m_assetManager(assetManager), m_fileSystem(fileSystem) {}

	bool SceneSerializer::Save(const std::string& virtualPath)
	{
		DAN_CORE_ASSERT(m_fileSystem != nullptr, "FileSystem invalido no SceneSerializer!");

		json sceneJson;
		sceneJson["Scene"] = "MainLevel";
		json entitiesArray = json::array();

		auto allEntities = m_registry.view<entt::entity>();

		for (auto entityID : allEntities)
		{
			json entityJson;
			entityJson["ID"] = static_cast<uint32_t>(entityID);

			// Varredura Automática de Tipos Refletidos no EnTT
			for (auto [id, type] : entt::resolve())
			{
				std::string typeName(type.name());
				if (typeName.empty()) continue;

				if (auto getFunc = type.func("get"_hs); getFunc)
				{
					entt::any componentInstance = getFunc.invoke({}, entt::forward_as_meta(m_registry), entityID);

					if (componentInstance) {
						json componentJson;
						SerializeMetaType(componentInstance, componentJson);
						entityJson[typeName] = componentJson;
					}
				}
			}

			// Injeta os componentes complexos com alocação manual
			SaveComplexComponents(m_registry, entityID, entityJson);

			entitiesArray.push_back(entityJson);
		}

		sceneJson["Entities"] = entitiesArray;

		std::filesystem::path fullPath = m_fileSystem->GetRootPath() / virtualPath;
		if (fullPath.has_parent_path()) {
			std::filesystem::create_directories(fullPath.parent_path());
		}

		std::ofstream outFile(fullPath);
		if (!outFile.is_open()) {
			DAN_ENGINE_ERROR("SceneSerializer: Falha ao abrir o arquivo para escrita em {0}", fullPath.string());
			return false;
		}

		outFile << sceneJson.dump(4);
		outFile.close();

		DAN_ENGINE_INFO("Cena salva com sucesso em: {0}", virtualPath);
		return true;
	}

	bool SceneSerializer::Load(const std::string& virtualPath)
	{
		std::string jsonString = m_fileSystem->ReadTextFile(virtualPath);
		if (jsonString.empty()) {
			DAN_ENGINE_ERROR("Falha ao carregar arquivo de nivel ou arquivo vazio: {0}", virtualPath);
			return false;
		}

		json data;
		try {
			data = json::parse(jsonString);
		}
		catch (const json::parse_error& e) {
			DAN_ENGINE_ERROR("Erro de sintaxe JSON no nivel '{0}': {1}", virtualPath, e.what());
			return false;
		}

		DAN_ENGINE_INFO("Carregando Nivel: {0}", data.value("Scene", "Unnamed"));
		m_registry.clear();

		const auto& entities = data["Entities"];
		if (!entities.is_array()) return true;

		for (const auto& entityData : entities)
		{
			auto entity = m_registry.create();

			// Desserialização Automática de Componentes Refletidos
			for (auto [id, type] : entt::resolve())
			{
				if (type.name() == nullptr) {
					DAN_ENGINE_WARN("Tipo refletido sem nome encontrado, ignorando...");
					continue;
				}

				std::string typeName(type.name());

				if (entityData.contains(typeName))
				{
					//entt::any newComponent = type.construct();
					entt::meta_any newComponent = type.construct();

					if (newComponent) {
						DeserializeMetaType(entityData[typeName], newComponent);

						if (auto assignFunc = type.func("assign"_hs); assignFunc)
						{
							assignFunc.invoke(
								{},
								entt::forward_as_meta(m_registry),
								entity,
								newComponent
							);
						}
					}
				}
			}

			// Carrega dependências complexas (Mesh, Terrain, Collider)
			LoadComplexComponents(entityData, m_registry, entity, m_assetManager, m_fileSystem);
		}

		DAN_ENGINE_INFO("Nivel '{0}' carregado com sucesso!", virtualPath);
		return true;
	}

} // namespace Dan