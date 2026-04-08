#pragma once

#include "glmcommon.hpp"
#include "RenderableMesh.hpp"
#include <vector>



struct TransformComponent 
{
    glm::vec3 pos{ 0.0f, 0.0f, 0.0f };
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    float rotY{ 0.0f };
};


struct LinearVelocityComponent
{
    glm::vec3 velocity{ 0.0f, 0.0f, 0.0f };
};


struct MeshComponent
{
    std::shared_ptr<eeng::RenderableMesh> mesh;
};


struct PlayerControllerComponent
{
    float speed = 10.0f;
};


struct NPCController
{
    float speed = 2.0f;
    std::vector<glm::vec3> waypoints;
    int currentWaypointIndex = 0;
};