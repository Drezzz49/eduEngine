#include "System.h"
#include "glmcommon.hpp"
#include "Component.hpp"



namespace System
{
    void Movement(entt::registry& reg, float deltaTime) {
        auto view = reg.view<TransformComponent, LinearVelocityComponent>(); //hittar alla entitys som har transform och velocity
        for (auto entity : view) {
            auto& transform = view.get<TransformComponent>(entity); //hämtar transform
            auto& vel = view.get<LinearVelocityComponent>(entity); //hämtar velocity
            transform.pos += vel.velocity * deltaTime;
        }
    }

    void PlayerController(entt::registry& reg, InputManagerPtr input) {
        auto view = reg.view<PlayerControllerComponent, LinearVelocityComponent>(); //hittar alla entitys som har playercontroller och velocity
        for (auto entity : view) {
            auto& playerCtrl = view.get<PlayerControllerComponent>(entity); //hämtar playercontroller, som bara är en fart
            auto& vel = view.get<LinearVelocityComponent>(entity); //hämtar velocity

            glm::vec3 moveDir{ 0, 0, 0 };
            //inputs ijkl iställer för wasd
            if (input->IsKeyPressed(eeng::InputManager::Key::I)) moveDir.z -= 1.0f;
            if (input->IsKeyPressed(eeng::InputManager::Key::K)) moveDir.z += 1.0f;
            if (input->IsKeyPressed(eeng::InputManager::Key::J)) moveDir.x -= 1.0f;
            if (input->IsKeyPressed(eeng::InputManager::Key::L)) moveDir.x += 1.0f;

            vel.velocity = moveDir * playerCtrl.speed;
        }
    }

    void Render(entt::registry& reg, std::shared_ptr<eeng::ForwardRenderer> renderer, std::shared_ptr<ShapeRendering::ShapeRenderer> shapeRenderer, bool drawSkeleton) {

        auto view = reg.view<TransformComponent, MeshComponent>(); //hittar alla entitys som har transform och mesh
        for (auto entity : view) {
            auto& transform = view.get<TransformComponent>(entity); //hämtar transform
            auto& meshComp = view.get<MeshComponent>(entity); //hämtar mesh

            if (meshComp.mesh) {
                if (reg.all_of<AnimationComponent>(entity)) //om entityn har animation
                {
                    auto& anim = reg.get<AnimationComponent>(entity);//hämtar animation
                    if (anim.useLayering)
                    {
                        eeng::AnimationBranchDesc filter;
                        filter.root_node_name = anim.layerRoot; //namnet på benet som ska vara root för den animation 2
                        filter.mode = eeng::AnimationBranchDesc::Mode::IncludeSubtree; //alla ben under det benet i animation 2

                        meshComp.mesh->animateBlend(
                            anim.primaryAnimClipIndex,
                            anim.secondaryAnimClipIndex,
                            anim.time,
                            anim.time,
                            filter);
                    }
                    else {
                        //standard blend utan layering
                        meshComp.mesh->animateBlend(
                            anim.primaryAnimClipIndex,
                            anim.secondaryAnimClipIndex,
                            anim.time,
                            anim.time,
                            anim.blendFactor);
                    }
                }

                glm::mat4 modelMatrix = glm_aux::TRS(transform.pos, transform.rotY, { 0, 1, 0 }, transform.scale);
                renderer->renderMesh(meshComp.mesh, modelMatrix); //ritar ut 

                //gizmo sklett
                if (drawSkeleton) {
                    float axisLen = 1.0f;

                    for (int i = 0; i < meshComp.mesh->boneMatrices.size(); ++i) //characterMesh blir till meshComp.mesh
                    {
                        auto IBinverse = glm::inverse(meshComp.mesh->m_bones[i].inversebind_tfm);
                        //characterWorldMatrix3 * characterMesh (från ) blir till modelMatrix * meshComp
                        glm::mat4 global = modelMatrix * meshComp.mesh->boneMatrices[i] * IBinverse;
                        glm::vec3 pos = glm::vec3(global[3]);

                        glm::vec3 right = glm::vec3(global[0]); // X
                        glm::vec3 up = glm::vec3(global[1]); // Y
                        glm::vec3 fwd = glm::vec3(global[2]); // Z

                        shapeRenderer->push_states(ShapeRendering::Color4u::Red);
                        shapeRenderer->push_line(pos, pos + axisLen * right);

                        shapeRenderer->push_states(ShapeRendering::Color4u::Green);
                        shapeRenderer->push_line(pos, pos + axisLen * up);

                        shapeRenderer->push_states(ShapeRendering::Color4u::Blue);
                        shapeRenderer->push_line(pos, pos + axisLen * fwd);

                        shapeRenderer->pop_states<ShapeRendering::Color4u>();
                        shapeRenderer->pop_states<ShapeRendering::Color4u>();
                        shapeRenderer->pop_states<ShapeRendering::Color4u>();

                    }
                }

            }
        }
    }

    void NPCController(entt::registry& reg, float deltaTime, float TargetTolerance) {
        auto view = reg.view<::NPCController, TransformComponent, LinearVelocityComponent>(); //hitta entitys med npccontroller, tarsnform och velocity
        for (auto entity : view) {
            auto& npc = view.get<::NPCController>(entity);
            auto& transform = view.get<TransformComponent>(entity);
            auto& vel = view.get<LinearVelocityComponent>(entity);

            if (npc.waypoints.empty()) continue; //om vi inte har några waypoints

            glm::vec3 target = npc.waypoints[npc.currentWaypointIndex];
            glm::vec3 toTarget = target - transform.pos;

            if (glm::length(toTarget) < TargetTolerance) { // Om nära nog, byt waypoint
                if (npc.currentWaypointIndex <= npc.waypoints.size())
                {
                    npc.currentWaypointIndex++; //nästa
                }
                if (npc.currentWaypointIndex >= npc.waypoints.size())
                {
                    npc.currentWaypointIndex = 0; //börja om
                }

				//kolla om entityn har en observer och notifyar den i så fall
                if (reg.all_of<Source>(entity))
                {
                    auto& sourceComponent = reg.get<Source>(entity);
                    sourceComponent.Notify(entity, Events::NPC_REACHED_WAYPOINT);
                }

            }
            else {
                vel.velocity = glm::normalize(toTarget) * npc.speed;
            }
        }
    }


    void Animation(entt::registry& reg, float deltaTime) {
        auto view = reg.view<AnimationComponent>(); //hitta alla som har animation
        for (auto entity : view) {
            auto& anim = view.get<AnimationComponent>(entity);
            anim.time += deltaTime * anim.speed; //öka tiden för animationen
        }
    }
}