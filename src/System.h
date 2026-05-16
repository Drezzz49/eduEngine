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
}

