#pragma once

#include "glmcommon.hpp"
#include "RenderableMesh.hpp"
#include <vector>
#include <algorithm>
#include <entt/entt.hpp>



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
    int primaryAnimClipIndex = 0; //va baseAnimationIndex
    int secondaryAnimClipIndex = 0; //va secondaryAnimationIndex
	float blendFactor = 0.0f;
    bool useLayering = false;
    float time = 0.0f;
    float speed = 1.0f;
    std::string layerRoot = "mixamorig:Spine";
};




//alla events 
enum class Events : std::uint8_t
{
    NPC_REACHED_WAYPOINT,
    GUI_BUTTON_CLICKED
};


//Observer Pattern 
class Observer //game klassen ärver från observer, och kan då ta emot events
{
public:
    virtual ~Observer() {};
    virtual void OnNotify(entt::entity entity, Events event) = 0;
};

struct Source
{
    //dynamic array of observers instead of 256
    std::vector<Observer*> _observers;

    //add
    void AddObserver(Observer* observer)
    {
        _observers.push_back(observer);
    }

    //remove
    void RemoveObserver(Observer* observer)
    {
        _observers.erase(std::remove(_observers.begin(), _observers.end(), observer), _observers.end());
    }

	//notify the observers of an event
    void Notify(entt::entity entity, Events event)
    {
        for (Observer* observer : _observers) 
        {
            if (observer != nullptr)
            {
                observer->OnNotify(entity, event);
            }
        }
    }
};







//Event Queue Pattern
struct PlayMessage
{
    entt::entity sender;
    Events event;
};


class EventQueueManager
{
private:
	std::vector<PlayMessage> currentQueue; //queue of events to process
	std::vector<PlayMessage> processingBuffer; //buffer to hold events while processing the current queue

public:
	//add event to the queue
    void Enqueue(entt::entity sender, Events event)
    {
        currentQueue.push_back({ sender, event });
    }

	//process the events in the queue
    void Flush(entt::registry& reg, std::string& outStatusText)
    {
        while (!currentQueue.empty())
        {
            // Swap queues
            processingBuffer = std::move(currentQueue);
            currentQueue.clear();

            for (const auto& message : processingBuffer)
            {
				//looking for specific events and updating accordingly
                if (message.event == Events::GUI_BUTTON_CLICKED)
                {
                    outStatusText = "Event Queue processed: GUI Button clicked temporally!";
                }
            }
            processingBuffer.clear();
        }
    }
};