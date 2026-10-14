#include <Dan/Common/Scene/SceneSerializer.hpp>
#include <Dan/Common/Assets/AssetManager.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Assets/Material.hpp>
#include <Dan/Common/Log.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Dan {

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

        auto entities = data["Entities"];
        if (entities.is_array())
        {
            for (auto entityData : entities)
            {
                auto entity = m_registry.create();

                // Desserializa Transform
                if (entityData.contains("Transform"))
                {
                    auto transformData = entityData["Transform"];
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
                    auto meshData = entityData["MeshComponent"];

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
                    auto camData = entityData["CameraComponent"];
                    CameraComponent camera;
                    camera.isPrimary = camData["IsPrimary"];
                    camera.fov = camData["Fov"];
                    camera.nearClip = camData["NearClip"];
                    camera.farClip = camData["FarClip"];
                    m_registry.emplace<CameraComponent>(entity, camera);
                }
            }
        }
        return true;
    }

    bool SceneSerializer::Save(const std::string& filepath)
    {
        // Implementação futura extraindo dados do m_registry iterando sobre as Views do EnTT.
        return true;
    }

} // namespace Dan