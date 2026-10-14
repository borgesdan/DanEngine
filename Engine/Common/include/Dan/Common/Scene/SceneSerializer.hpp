#pragma once
#include <string>
#include <entt/entt.hpp>
#include "../Assets/AssetManager.hpp"
#include "../IO/FileSystem.hpp"

namespace Dan {

    class SceneSerializer
    {
    public:
        SceneSerializer(entt::registry& registry, AssetManager* assetManager, FileSystem* fileSystem);

        bool Load(const std::string& filepath);
        bool Save(const std::string& filepath);

    private:
        entt::registry& m_registry;
        AssetManager* m_assetManager;
        FileSystem* m_fileSystem;
    };

} // namespace Dan