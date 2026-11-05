#include <Dan/Common/Scene/SceneSerializer.hpp>
#include <Dan/Common/Assets/AssetManager.hpp>
#include <Dan/Common/IO/FileSystem.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>

// Componentes do ECS
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <Dan/Common/Components/PhysicsComponents.hpp>

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

                // Desserializa Transform
                if (entityData.contains("Transform"))
                {
                    const auto& transformData = entityData["Transform"];
                    Math::Transform transform;

                    transform.translation = { transformData["Translation"][0], transformData["Translation"][1], transformData["Translation"][2] };

                    // Conversão de Euler para Quaternion
                    Math::Vec3 eulerRotation = { transformData["Rotation"][0], transformData["Rotation"][1], transformData["Rotation"][2] };
                    transform.rotation = glm::quat(glm::radians(eulerRotation));

                    transform.scale = { transformData["Scale"][0], transformData["Scale"][1], transformData["Scale"][2] };

                    m_registry.emplace<Math::Transform>(entity, transform);
                }

                // Desserializa MeshComponent
                if (entityData.contains("MeshComponent"))
                {
                    const auto& meshData = entityData["MeshComponent"];

                    auto mesh = m_assetManager->Load<Mesh>(meshData["MeshPath"]);
                    auto vs = m_assetManager->Load<Shader>(meshData["VertexShader"]);
                    auto fs = m_assetManager->Load<Shader>(meshData["FragmentShader"]);

                    if (mesh && vs && fs) {
                        auto material = std::make_shared<Material>(vs, fs);
                        m_registry.emplace<MeshComponent>(entity, MeshComponent{ mesh, material });
                    }
                    else {
                        DAN_ENGINE_WARN("Falha ao carregar dependências visuais da entidade.");
                    }
                }

                // Desserializa CameraComponent
                if (entityData.contains("CameraComponent"))
                {
                    const auto& camData = entityData["CameraComponent"];
                    CameraComponent camera;
                    camera.isPrimary = camData["IsPrimary"];
                    camera.fov = camData["Fov"];
                    camera.nearClip = camData["NearClip"];
                    camera.farClip = camData["FarClip"];
                    m_registry.emplace<CameraComponent>(entity, camera);
                }

                // RigidBodyComponent
                if (entityData.contains("RigidBodyComponent"))
                {
                    const auto& rbData = entityData["RigidBodyComponent"];
                    RigidBodyComponent rb;
                    rb.motionType = StringToMotionType(rbData.value("MotionType", "Dynamic"));
                    rb.mass = rbData.value("Mass", 1.0f);
                    rb.allowSleeping = rbData.value("AllowSleeping", true);
                    m_registry.emplace<RigidBodyComponent>(entity, rb);
                }

                // 5. ColliderComponent
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

                    m_registry.emplace<ColliderComponent>(entity, collider);
                }
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

            // A. Serializa Math::Transform
            if (auto* transform = m_registry.try_get<Math::Transform>(entityID))
            {
                json transformJson;
                transformJson["Translation"] = SerializeVec3(transform->translation);
                transformJson["Rotation"] = SerializeQuat(transform->rotation);
                transformJson["Scale"] = SerializeVec3(transform->scale);
                entityJson["Transform"] = transformJson;
            }

            // B. Serializa CameraComponent
            if (auto* camera = m_registry.try_get<CameraComponent>(entityID))
            {
                json camJson;
                camJson["IsPrimary"] = camera->isPrimary;
                camJson["Fov"] = camera->fov;
                camJson["NearClip"] = camera->nearClip;
                camJson["FarClip"] = camera->farClip;
                camJson["Yaw"] = camera->yaw;
                camJson["Pitch"] = camera->pitch;
                entityJson["CameraComponent"] = camJson;
            }

            // C. Serializa DirectionalLightComponent
            if (auto* light = m_registry.try_get<DirectionalLightComponent>(entityID))
            {
                json lightJson;
                lightJson["Direction"] = SerializeVec3(light->direction);
                lightJson["Color"] = SerializeVec3(light->color);
                lightJson["Intensity"] = light->intensity;
                entityJson["DirectionalLightComponent"] = lightJson;
            }

            // D. Serializa MeshComponent e o Material associado
            if (auto* meshComp = m_registry.try_get<MeshComponent>(entityID))
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

            // D. RigidBodyComponent
            if (auto* rb = m_registry.try_get<RigidBodyComponent>(entityID))
            {
                json rbJson;
                rbJson["MotionType"] = MotionTypeToString(rb->motionType);
                rbJson["Mass"] = rb->mass;
                rbJson["AllowSleeping"] = rb->allowSleeping;
                entityJson["RigidBodyComponent"] = rbJson;
            }

            // E. ColliderComponent
            if (auto* collider = m_registry.try_get<ColliderComponent>(entityID))
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