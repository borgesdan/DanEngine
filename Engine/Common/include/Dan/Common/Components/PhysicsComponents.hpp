#pragma once

#include <Dan/Common/Math/MathTypes.hpp>
#include <cstdint>
#include <variant>
#include <vector>

namespace Dan {

    enum class MotionType : uint8_t
    {
        Static,     // Objeto imóvel (chão, paredes)
        Kinematic,  // Movido por código/animação, não responde a forças
        Dynamic     // Afetado por gravidade, colisões e forças
    };

    // Formas Geométricas de Colisão (Data POD)
    struct BoxColliderShape
    {
        Math::Vec3 halfExtents{ 0.5f, 0.5f, 0.5f };
    };

    struct SphereColliderShape
    {
        float radius = 0.5f;
    };

    struct CapsuleColliderShape
    {
        float halfHeight = 0.5f;
        float radius = 0.25f;
    };

    struct MeshColliderShape
    {
        std::vector<Math::Vec3> vertices;
        std::vector<uint32_t> indices;
        bool isConvex = true;
    };

    using ColliderShapeData = std::variant<BoxColliderShape, SphereColliderShape, CapsuleColliderShape, MeshColliderShape>;

    // Componente de Colisor
    struct ColliderComponent
    {
        ColliderShapeData shape = BoxColliderShape{};
        Math::Vec3 centerOffset{ 0.0f, 0.0f, 0.0f };
        bool isTrigger = false;
        float friction = 0.5f;
        float restitution = 0.1f; // Coeficiente de restituição (pulo/quique)
    };

    // Componente de Corpo Rígido
    struct RigidBodyComponent
    {
        MotionType motionType = MotionType::Dynamic;
        float mass = 1.0f;
        bool allowSleeping = true;

        // Identificador/Handle opaco do corpo no mundo de física interno
        uint32_t runtimeBodyID = 0xFFFFFFFF;
        bool isInitialized = false;
    };

} // namespace Dan