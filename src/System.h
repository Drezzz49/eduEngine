#pragma once
#pragma once
#include <entt/entt.hpp>
#include <memory>
#include "InputManager.hpp"
#include "ForwardRenderer.hpp"
#include "ShapeRenderer.hpp"


namespace System
{
    void Movement(entt::registry& reg, float deltaTime);

    void PlayerController(entt::registry& reg, InputManagerPtr input);

    void Render(entt::registry& reg,
        std::shared_ptr<eeng::ForwardRenderer> renderer,
        std::shared_ptr<ShapeRendering::ShapeRenderer> shapeRenderer,
        bool drawSkeleton);

    void NPCController(entt::registry& reg, float deltaTime, float TargetTolerance);

    void Animation(entt::registry& reg, float deltaTime);


	// Collision detection functions
    bool TestSphereSphere(glm::vec3 centerA, float radiusA, glm::vec3 centerB, float radiusB);

    bool TestAABBAABB(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB);

    void CollisionSystem(entt::registry& reg, ShapeRendering::ShapeRenderer& shapeRenderer);

    bool CheckNarrowPhaseCollision(entt::registry& reg, entt::entity entA, entt::entity entB);

    void ResolveCollision(entt::registry& reg, entt::entity entA, entt::entity entB);
}

