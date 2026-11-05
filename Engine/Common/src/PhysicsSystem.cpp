#include <Dan/Common/Physics/PhysicsSystem.hpp>
#include <Dan/Common/Components/PhysicsComponents.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>

// Headers da Jolt Physics isolados no arquivo fonte
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

namespace Dan {

    namespace Layers {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    }

    namespace BroadPhaseLayers {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr uint32_t NUM_LAYERS = 2;
    }

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        }

        uint32_t GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            return mObjectToBroadPhase[inLayer];
        }

    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
        {
            if (inLayer1 == Layers::NON_MOVING)
                return inLayer2 == BroadPhaseLayers::MOVING;
            return true;
        }
    };

    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override
        {
            if (inLayer1 == Layers::NON_MOVING && inLayer2 == Layers::NON_MOVING)
                return false;
            return true;
        }
    };

    struct PhysicsSystem::Impl
    {
        JPH::TempAllocatorImpl* tempAllocator = nullptr;
        JPH::JobSystemThreadPool* jobSystem = nullptr;
        BPLayerInterfaceImpl broadPhaseLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl objectVsBroadphaseFilter;
        ObjectLayerPairFilterImpl objectVsObjectFilter;
        JPH::PhysicsSystem physicsSystem;

        Impl()
        {
            JPH::RegisterDefaultAllocator();
            JPH::Factory::sInstance = new JPH::Factory();
            JPH::RegisterTypes();

            tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
            jobSystem = new JPH::JobSystemThreadPool(
                JPH::cMaxPhysicsJobs,
                JPH::cMaxPhysicsBarriers,
                std::thread::hardware_concurrency() - 1
            );

            constexpr uint32_t maxBodies = 2048;
            constexpr uint32_t numBodyMutexes = 0;
            constexpr uint32_t maxBodyPairs = 2048;
            constexpr uint32_t maxContactConstraints = 2048;

            physicsSystem.Init(
                maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints,
                broadPhaseLayerInterface, objectVsBroadphaseFilter, objectVsObjectFilter
            );

            physicsSystem.SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
        }

        ~Impl()
        {
            delete jobSystem;
            delete tempAllocator;
            JPH::UnregisterTypes();
            delete JPH::Factory::sInstance;
            JPH::Factory::sInstance = nullptr;
        }
    };

    PhysicsSystem::PhysicsSystem()
        : m_impl(std::make_unique<Impl>())
    {
        DAN_ENGINE_INFO("PhysicsSystem (Jolt Physics) inicializado com sucesso.");
    }

    PhysicsSystem::~PhysicsSystem() = default;

    void PhysicsSystem::Update(entt::registry& registry, const EngineContext& context)
    {
        // 1. Instancia corpos novos criados no EnTT
        SyncConstructedBodies(registry);

        // 2. Avança a simulação e atualiza os Transforms no ECS
        if (context.time.elapsedGameTime > 0.0f)
        {
            StepSimulationAndSyncTransforms(registry, context.time.elapsedGameTime);
        }
    }

    void PhysicsSystem::SyncConstructedBodies(entt::registry& registry)
    {
        auto& bodyInterface = m_impl->physicsSystem.GetBodyInterface();

        auto view = registry.view<Math::Transform, RigidBodyComponent, ColliderComponent>();
        for (auto [entity, transform, rb, collider] : view.each())
        {
            if (rb.isInitialized) continue;

            // Criação das formas geométricas (Shapes)
            JPH::RefConst<JPH::Shape> shape;

            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, BoxColliderShape>)
                {
                    shape = new JPH::BoxShape(JPH::Vec3(
                        arg.halfExtents.x * transform.scale.x,
                        arg.halfExtents.y * transform.scale.y,
                        arg.halfExtents.z * transform.scale.z));
                }
                else if constexpr (std::is_same_v<T, SphereColliderShape>)
                {
                    shape = new JPH::SphereShape(arg.radius * transform.scale.x);
                }
                else if constexpr (std::is_same_v<T, CapsuleColliderShape>)
                {
                    shape = new JPH::CapsuleShape(arg.halfHeight * transform.scale.y, arg.radius * transform.scale.x);
                }
                }, collider.shape);

            if (!shape) continue;

            // Configuração do tipo de corpo
            JPH::EMotionType joltMotionType = JPH::EMotionType::Dynamic;
            JPH::ObjectLayer layer = Layers::MOVING;

            if (rb.motionType == MotionType::Static)
            {
                joltMotionType = JPH::EMotionType::Static;
                layer = Layers::NON_MOVING;
            }
            else if (rb.motionType == MotionType::Kinematic)
            {
                joltMotionType = JPH::EMotionType::Kinematic;
            }

            JPH::Vec3 position(transform.translation.x, transform.translation.y, transform.translation.z);
            JPH::Quat rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);

            JPH::BodyCreationSettings bodySettings(shape, position, rotation, joltMotionType, layer);
            bodySettings.mFriction = collider.friction;
            bodySettings.mRestitution = collider.restitution;

            JPH::Body* body = bodyInterface.CreateBody(bodySettings);
            if (body)
            {
                bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);
                rb.runtimeBodyID = body->GetID().GetIndexAndSequenceNumber();
                rb.isInitialized = true;
            }
        }
    }

    void PhysicsSystem::StepSimulationAndSyncTransforms(entt::registry& registry, float deltaTime)
    {
        constexpr int collisionSteps = 1;
        m_impl->physicsSystem.Update(deltaTime, collisionSteps, m_impl->tempAllocator, m_impl->jobSystem);

        auto& bodyInterface = m_impl->physicsSystem.GetBodyInterface();

        auto view = registry.view<Math::Transform, const RigidBodyComponent>();
        for (auto [entity, transform, rb] : view.each())
        {
            if (!rb.isInitialized || rb.motionType == MotionType::Static) continue;

            JPH::BodyID bodyID(rb.runtimeBodyID);
            if (bodyInterface.IsActive(bodyID))
            {
                JPH::Vec3 pos = bodyInterface.GetPosition(bodyID);
                JPH::Quat rot = bodyInterface.GetRotation(bodyID);

                transform.translation = Math::Vec3(pos.GetX(), pos.GetY(), pos.GetZ());
                transform.rotation = Math::Quat(rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW());
            }
        }
    }

} // namespace Dan