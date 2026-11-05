#pragma once

#include <Dan/Common/System/EngineContext.hpp>
#include <entt/entt.hpp>
#include <memory>

namespace Dan {

    class PhysicsSystem
    {
    public:
        PhysicsSystem();
        ~PhysicsSystem();

        PhysicsSystem(const PhysicsSystem&) = delete;
        PhysicsSystem& operator=(const PhysicsSystem&) = delete;

        // Chamado a cada quadro no laço do EngineCore
        void Update(entt::registry& registry, const EngineContext& context);

    private:
        void SyncConstructedBodies(entt::registry& registry);
        void SyncConstructedTerrain(entt::registry& registry);
        void StepSimulationAndSyncTransforms(entt::registry& registry, float deltaTime);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace Dan