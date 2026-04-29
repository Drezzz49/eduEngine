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


struct AnimationComponent
{
    int baseAnimationIndex = 0;
    int secondaryAnimationIndex = 1;
    float blendFactor = 0.0f;
    bool useLayering = false;
    float time = 0.0f;
    float speed = 1.0f;
    std::string layerRoot = "mixamorig:Spine";
};