#include <iostream>
#include "Entity/Entity.hpp"
#include "Log.hpp"

Entity::Entity(EntityManager& manager): entityManager(manager) 
{
    this->isActive = true;
}

Entity::Entity(EntityManager& manager, std::string name): entityManager(manager), name(name)
{
    this->isActive = true;
}

Entity::Entity(EntityManager& manager, std::string name, Layer layer): entityManager(manager), name(name), layer_(layer)
{
    this->isActive = true;
}

void Entity::Update(float deltaTime) 
{   
    if (isActive == false)
        return;

    for (auto& component: components) 
    {
        component->Update(deltaTime);
    }
}

void Entity::Render() 
{
    if (isActive == false)
        return;
        
    for (auto& component: components) 
    {
        component->Render();
    }
}

void Entity::Destroy() 
{
    this->isActive = false;
}

void Entity::Activate() 
{
    this->isActive = true;
}

bool Entity::IsActive() const 
{
    return this->isActive;
}

void Entity::ListAllComponents() const 
{
    for (auto mapElement: componentTypes)
        Logger::Log(logging::trivial::debug, log_location, "      Component<", mapElement.first->name() , ">");

}