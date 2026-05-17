#include "System.h"
#include "glmcommon.hpp"
#include "Component.hpp"

namespace System
{

    struct BVHSphere {
        glm::vec3 center;
        float radius;
    };


    struct SphereNode {
        BVHSphere collisionRepresentation;
        SphereNode* leftChild = nullptr;
        SphereNode* rightChild = nullptr;

        entt::entity entity = entt::null;

        bool isLeaf() const {
			//om vi inte har några barn så är det en leaf
            return leftChild == nullptr && rightChild == nullptr;
        }
    };





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




    bool TestSphereSphere(glm::vec3 centerA, float radiusA, glm::vec3 centerB, float radiusB)
    {
        glm::vec3 d = centerA - centerB;
        float distanceSq = glm::dot(d, d);
        float radiusSum = radiusA + radiusB;

		//om avståndet är mindre än summan av radierna så kolliderar de
        return distanceSq <= (radiusSum * radiusSum);
    }



    bool TestAABBAABB(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB)
    {
        //överlapp i x
        if (maxA.x < minB.x || minA.x > maxB.x) return false;
        //överlapp i y
        if (maxA.y < minB.y || minA.y > maxB.y) return false;
        //överlapp i z
        if (maxA.z < minB.z || minA.z > maxB.z) return false;

		//överlapp på alla axlar ger kollision
        return true;
    }




    SphereNode* BuildSphereTree(entt::registry& reg, const std::vector<entt::entity>& entities)
    {
        if (entities.empty()) return nullptr;

        SphereNode* node = new SphereNode();

        //om det bara finns en kvar är det ett löv
        if (entities.size() == 1)
        {
            node->entity = entities[0];

			//lövets bvh sfär blir samma som den riktiga sfären
            auto& trans = reg.get<TransformComponent>(node->entity);
            auto& sphere = reg.get<SphereColliderComponent>(node->entity);
            node->collisionRepresentation.center = trans.pos + sphere.center;
            node->collisionRepresentation.radius = sphere.radius;

            return node;
        }

		//när det finns mer än en entity, dela upp i två grupper
		//vi hittar en medianpunkt, så det blir medelpunkten för en stor sfär som omsluter alla entities
        glm::vec3 centerSum(0.0f);
        for (auto ent : entities)
        {
            centerSum += reg.get<TransformComponent>(ent).pos;
        }
        glm::vec3 averageCenter = centerSum / (float)entities.size();

		//vi mäter vilket avstånd som är längst från medelpunkten och använder det som radie
        float maxRadius = 0.0f;
        for (auto ent : entities)
        {
            auto& trans = reg.get<TransformComponent>(ent);
            auto& sphere = reg.get<SphereColliderComponent>(ent);
            float dist = glm::distance(averageCenter, trans.pos + sphere.center) + sphere.radius;
            if (dist > maxRadius) maxRadius = dist;
        }

        node->collisionRepresentation.center = averageCenter;
        node->collisionRepresentation.radius = maxRadius;

        //dela upp entitiesen i två grupper
        std::vector<entt::entity> leftGroup;
        std::vector<entt::entity> rightGroup;

		//delar upp baserat på index, borde lösa på ett bättre sätt egentligen
        size_t half = entities.size() / 2;
        for (size_t i = 0; i < entities.size(); ++i)
        {
            if (i < half) {
                leftGroup.push_back(entities[i]);
            }  
            else {
                rightGroup.push_back(entities[i]);
            }   
        }

		//bygger vidare rekursivt för varje grupp
        node->leftChild = BuildSphereTree(reg, leftGroup);
        node->rightChild = BuildSphereTree(reg, rightGroup);

        return node;
    }


    void TraverseSphereTree(entt::registry& reg, SphereNode* nodeA, SphereNode* nodeB)
    {
        //om någon av noderna inte finns eller är samma
        if (!nodeA || !nodeB || nodeA == nodeB) {
            return;
        }

		//kollar om de två bvh nodernas sfärer kolliderar
        if (!TestSphereSphere(nodeA->collisionRepresentation.center, nodeA->collisionRepresentation.radius,
            nodeB->collisionRepresentation.center, nodeB->collisionRepresentation.radius))
        {
            return;
        }

        //om båda är löv så kan det finns en kollision
        if (nodeA->isLeaf() && nodeB->isLeaf())
        {
			//kollar om de riktiga sfärerna kolliderar
            if (CheckNarrowPhaseCollision(reg, nodeA->entity, nodeB->entity))
            {
                ResolveCollision(reg, nodeA->entity, nodeB->entity);
            }
            return;
        }

		//om en eller båda är grenar går vi djupare på den som har störst radie först
        if (nodeB->isLeaf() || (!nodeA->isLeaf() && nodeA->collisionRepresentation.radius >= nodeB->collisionRepresentation.radius))
        {
            TraverseSphereTree(reg, nodeA->leftChild, nodeB);
            TraverseSphereTree(reg, nodeA->rightChild, nodeB);
        }
        else
        {
            TraverseSphereTree(reg, nodeA, nodeB->leftChild);
            TraverseSphereTree(reg, nodeA, nodeB->rightChild);
        }
    }


	//tar bort trädet och frigör minnet
    void FreeSphereTree(SphereNode* node)
    {
        if (!node) {
            return;
        }
          
        FreeSphereTree(node->leftChild);
        FreeSphereTree(node->rightChild);
        delete node;
    }



    void CollisionSystem(entt::registry& reg, ShapeRendering::ShapeRenderer& shapeRenderer)
    {
        //gå igenom alla sfärer (rita)
        auto sphereView = reg.view<TransformComponent, SphereColliderComponent>();
        for (auto entity : sphereView)
        {
            auto& transform = sphereView.get<TransformComponent>(entity);
            auto& sphere = sphereView.get<SphereColliderComponent>(entity);

			//vi räknar ut världens position för sfären
            glm::vec3 worldCenter = transform.pos + sphere.center;

            //flyttar till worldCenter och skalar till sfärens radie
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), worldCenter);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(sphere.radius));

			//Rita ut sfären
            shapeRenderer.push_states(ShapeRendering::Color4u{ 0xffff0000 });
            shapeRenderer.push_states(modelMatrix);
            shapeRenderer.push_sphere_wireframe(1.0f, 1.0f);

			//poppa de tillstånd vi pushade
            shapeRenderer.pop_states<glm::mat4>();
            shapeRenderer.pop_states<ShapeRendering::Color4u>();
        }

		//gå igenom alla AABBs (rita)
        auto aabbView = reg.view<TransformComponent, AABBColliderComponent>();
        for (auto entity : aabbView)
        {
            auto& transform = aabbView.get<TransformComponent>(entity);
            auto& aabb = aabbView.get<AABBColliderComponent>(entity);

			//räkna ut världens min och max för AABB
            glm::vec3 worldMin = aabb.min + transform.pos;
            glm::vec3 worldMax = aabb.max + transform.pos;

            //rita ut AABB i världen så vi ser den
            shapeRenderer.push_states(ShapeRendering::Color4u{ 0xffffffff });
            shapeRenderer.push_AABB(worldMin, worldMax);
            shapeRenderer.pop_states<ShapeRendering::Color4u>();
        }



        //kolla kollisioner
        auto view = reg.view<TransformComponent, SphereColliderComponent, AABBColliderComponent>();

        std::vector<entt::entity> allEntities;
        for (auto entity : view) //samlar alla entites
        {
            allEntities.push_back(entity);
        }

        //bygg trädet (varje frame)
        SphereNode* rootNode = BuildSphereTree(reg, allEntities);

		//vi går igenom trädet och kollar kollisioner
        if (rootNode)
        {
			//kollisioner mellan vänster och höger underträd
            if (rootNode->leftChild && rootNode->rightChild) {
                TraverseSphereTree(reg, rootNode->leftChild, rootNode->rightChild);
            }

			//kollisioner inom vänster underträd
            if (rootNode->leftChild && !rootNode->leftChild->isLeaf()) {
                TraverseSphereTree(reg, rootNode->leftChild->leftChild, rootNode->leftChild->rightChild);
            }

			//kollisioner inom höger underträd
            if (rootNode->rightChild && !rootNode->rightChild->isLeaf()) {
                TraverseSphereTree(reg, rootNode->rightChild->leftChild, rootNode->rightChild->rightChild);
            }
        }

        //tar bort trädet
        FreeSphereTree(rootNode);


        //gamla kollisiontestet innan trädet
        //for (auto entA : view)
        //{
        //    for (auto entB : view)
        //    {
        //        if (entA == entB) continue; //jämför inte med sig själv

        //        //kollar om vi kolliderar
        //        if (CheckNarrowPhaseCollision(reg, entA, entB))
        //        {
        //            //separera
        //            ResolveCollision(reg, entA, entB);
        //        }
        //    }
        //}
    }



    bool CheckNarrowPhaseCollision(entt::registry& reg, entt::entity entA, entt::entity entB)
    {
        auto& transA = reg.get<TransformComponent>(entA);
        auto& sphereA = reg.get<SphereColliderComponent>(entA);
        auto& aabbA = reg.get<AABBColliderComponent>(entA);

        auto& transB = reg.get<TransformComponent>(entB);
        auto& sphereB = reg.get<SphereColliderComponent>(entB);
        auto& aabbB = reg.get<AABBColliderComponent>(entB);

        //trigger kan inte kollidera med trigger
        if (sphereA.isTrigger && sphereB.isTrigger)
        {
            return false;
        }

        //positioner för sfärer
        glm::vec3 centerA = transA.pos + sphereA.center;
        glm::vec3 centerB = transB.pos + sphereB.center;

        //Sphere-Sphere test
        if (!TestSphereSphere(centerA, sphereA.radius, centerB, sphereB.radius))
        {
            return false;
        }

        //om sfärerna krockade, testa aabb
        glm::vec3 minA = aabbA.min + transA.pos;
        glm::vec3 maxA = aabbA.max + transA.pos;
        glm::vec3 minB = aabbB.min + transB.pos;
        glm::vec3 maxB = aabbB.max + transB.pos;

        //AABB-AABB test
        return TestAABBAABB(minA, maxA, minB, maxB);
    }



    //lös kollisoner genom att flytta de ifrån varandra
    void ResolveCollision(entt::registry& reg, entt::entity entA, entt::entity entB)
    {
		//om någon av de är en trigger
		auto& sphereA = reg.get<SphereColliderComponent>(entA);
		auto& sphereB = reg.get<SphereColliderComponent>(entB);
        if (sphereA.isTrigger || sphereB.isTrigger)
        {
			//om a är triggern så skickar vi entA, annars entB, så vi lätt kan kolla vilken som blir triggad
            if (sphereA.isTrigger && reg.all_of<Source>(entA)) {
                reg.get<Source>(entA).Notify(entA, Events::TRIGGER_ENTERED);
            }
            //om b är triggern så skickar vi entB, annars entA
            if (sphereB.isTrigger && reg.all_of<Source>(entB)) {
                reg.get<Source>(entB).Notify(entB, Events::TRIGGER_ENTERED);
            }

            return;
        }


        auto& transA = reg.get<TransformComponent>(entA);
        auto& transB = reg.get<TransformComponent>(entB);

        glm::vec3 centerA = transA.pos + sphereA.center;
        glm::vec3 centerB = transB.pos + sphereB.center;

		//längden mellan mittpunkterna
        glm::vec3 dir = centerA - centerB;
        float dist = glm::length(dir);

        //om de är på samma plats
        if (dist == 0.0f){
			//flytta a lite
            dir = glm::vec3(1.0f, 0.0f, 0.0f); dist = 0.01f; 
        }

        float overlap = (sphereA.radius + sphereB.radius) - dist;

        if (overlap > 0.0f)
        {
			//flytta a och b ifrån varandra lika mycket
            glm::vec3 separationVec = glm::normalize(dir) * (overlap * 0.5f);

            transA.pos += separationVec;
            transB.pos -= separationVec;
        }
    }


}