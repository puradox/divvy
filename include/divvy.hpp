/*
 * Divvy, a simple component entity framework.
 *
 * Licensed under The MIT License (MIT)
 *
 * Copyright (c) 2015 Sam Balana <sambalana247@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef DIVVY_HPP
#define DIVVY_HPP

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace divvy {

// =====================================[ Entity ]=======================================

class World; // Forward declaration

/// An Entity represents a simple (unsigned) number
typedef std::uint_fast32_t EntityID;

class Entity
{
public:
    /**
     * Creates a null Entity
     */
    Entity() {}

    /**
     * Creates an Entity in the specified World
     * @param world     The world used to create an Entity
     */
    Entity(World& world);

    /**
     * Creates a clone of an Entity in the specified World
     * @param other     The Entity to clone
     */
    Entity(const Entity& other);

    /**
     * Creates a clone of an Entity in the specified World
     * @param world     The world used to create an Entity
     * @param other     The Entity to clone
     */
    Entity(World& world, const Entity& other);

    /**
     * Destructor
     */
    ~Entity();

    /**
     * Move operator, creates an exact duplicate of the other Entity
     * @param rhs   Entity to be duplicated
     */
    Entity& operator=(const Entity& rhs);

    /**
     * Assign a Component to this Entity
     * @returns reference to the Component assigned
     */
    template <class T, class ... Args>
    inline T& add(Args&& ... args);

    /**
     * Removes a Component from this Entity
     */
    template <class T>
    inline void remove();

    /**
     * Retreive a Component
     * @returns reference to the Component
     */
    template <class T>
    inline T& get();

    /**
     * Recreates an Entity in the specified World
     * @param world     The world used to create an Entity
     */
    inline void reset(World& world);

    /**
     * Recreates a clone of an Entity in the specified World
     * @param other     The Entity to clone
     */
    inline void reset(const Entity& other);

    /**
     * Recreates a clone of an Entity in the specified World
     * @param world     The world used to create an Entity
     * @param other     The Entity to clone
     */
    inline void reset(World& world, const Entity& other);

    /**
     * Remove an Entity from the World.
     */
    void destroy();

private:
    friend class World;
    friend std::ostream& operator<<(std::ostream& stream, const Entity& entity);

    /// Identification number to reference the Entity.
    EntityID m_id = 0;

    /// The World in which this Entity exists in.
    World* m_world = nullptr;
};

std::ostream& operator<<(std::ostream& stream, const Entity& entity)
{
    stream << entity.m_id;
    return stream;
}

// ===================================[ Component ]======================================

/**
 * Component is a base class that provides functionality for the entities
 * through the update function. Components can also be used to store
 * information about entites.
 */
class Component
{
public:
    /// Allow derived classes to have a destructor.
    virtual ~Component() {}

    /// Provide functionality to an Entity.
    virtual void update() = 0;

    /// Make a copy of the Component.
    virtual std::shared_ptr<Component> clone() const = 0;

protected:
    friend class World;

    /// The World that is using the Component.
    World* m_world;

    /// The EntityID that is assigned to the Component.
    EntityID m_entity;
};

// =====================================[ World ]========================================

/// Defines what a ComponentRegisry is, type-wise
typedef std::map<std::string, std::vector<std::shared_ptr<Component>>> ComponentRegistry;

/**
 * World is the heart of all Component operations, as it calls each Component's
 * update method. The creation of Entities and Components happen within a
 * world. Since Entities and Components are localized within a World, there
 * is the possibility to have multiple Worlds, each being different than the
 * other.
 */
class World
{
private:
    /// The local m_registry of Components types and the Entites that use them.
    ComponentRegistry m_registry;

    /// Number of Entities existing in the World.
    size_t m_slots = 0;

    /// An ordered queue in which Entities were deleted in.
    /// Serves the purpose of filling in gaps in memory where Entities were
    /// previously deleted.
    std::set<int> m_open;

public:
    /**
     * Registers a Component on the ComponentRegistry of the World.
     */
    template <class T>
    void registerComponent()
    {
        if (!validComponent<T>())
            throw std::runtime_error("Not a valid Component - class must derive from Component");

        // Register on the ComponentRegistry
        std::string name = typeid(T).name();
        m_registry.insert(std::make_pair(name, std::vector<std::shared_ptr<Component>>()));

        #ifdef DIVVY_DEBUG
        std::cout << "-- Registered Component Type: " << name << std::endl;
        #endif
    }

    /**
     * Update all the Components registered in the World
     */
    void update()
    {
        // Update all Components
        for (auto it = m_registry.begin(); it != m_registry.end(); it++)
            for (int i = 0; i < m_slots; i++)
                if (it->second.at(i) != nullptr && m_open.find(i) == m_open.end())
                    it->second.at(i)->update();
    }

    int slots() { return m_slots; }

private:
    friend class Entity;
    friend class Component;

    /**
     * Checks whether an Entity is nonexistent or null.
     * @returns true if existing, false otherwise
     */
    inline bool hasEntity(const Entity& entity)
    {
        if (m_open.find(entity.m_id) != m_open.end() || m_slots <= entity.m_id)
            return false;
        return true;
    }

    /**
     * Checks whether a Component is registered in the World.
     * @returns true if the Component is registered, false otherwise
     */
    template <class T>
    inline bool hasComponent()
    {
        if (m_registry.find(typeid(T).name()) == m_registry.end())
            return false;
        return true;
    }

    /**
     * Checks whether an Entity contains a specific Component.
     * @returns true if Entity contains the specific Component, false otherwise
     */
    template <class T>
    inline bool hasComponent(const Entity& entity)
    {
        std::string name = typeid(T).name();
        if (m_registry.find(name) == m_registry.end() || m_registry.at(name).at(entity.m_id) == nullptr)
            return false;
        return true;
    }

    /**
     * Checks whether a Component derives from the Component base class.
     * @returns true if valid, false otherwise
     */
    template <class T>
    inline bool validComponent()
    {
        if (!std::is_base_of<Component, T>::value && !std::is_abstract<T>::value)
            return false;
        return true;
    }

    /**
     * Create an Entity in the World.
     * @returns newly created Entity
     */
    EntityID addEntity()
    {
        if (m_open.size() != 0) // Free m_slots available
        {
            // Remove the open slot
            int index = *(m_open.begin());
            m_open.erase(m_open.begin());

            #ifdef DIVVY_DEBUG
            std::cout << "-- Added new Entity at #" << index << " (reused)" << std::endl;
            #endif

            return index;
        }
        else
        {
            // Resize ComponentRegistry
            for (auto it = m_registry.begin(); it != m_registry.end(); it++)
                it->second.resize(m_slots + 1);

            #ifdef DIVVY_DEBUG
            std::cout << "-- Added new Entity at #" << m_slots << std::endl;
            #endif

            // Add a slot for the new Entity
            return m_slots++;
        }
    }

    /**
     * Clone an Entity in the World.
     * @returns cloned Entity
     */
    EntityID addEntity(const World& world, const Entity& other)
    {
        EntityID id = addEntity();

        // Clone all Components of other Entity
        for (auto it = world.m_registry.begin(); it != world.m_registry.end(); it++)
        {
            if (it->second.at(other.m_id) != nullptr)
            {
                m_registry.at(it->first).at(id) = it->second.at(other.m_id)->clone();
                m_registry.at(it->first).at(id)->m_world = this;
                m_registry.at(it->first).at(id)->m_entity = id;
            }
        }

        return id;
    }

    /**
     * Assign a Component to an Entity
     * @returns reference to the Component assigned
     */
    template <class T, class ... Args>
    T& addComponent(const Entity& entity, Args&& ... args)
    {
        if (!validComponent<T>())
            throw std::runtime_error("Not a valid Component - class must derive from Component");

        if (!hasEntity(entity))
            throw std::runtime_error("Entity non-existent - call hasEntity() beforehand");

        if (!hasComponent<T>())
            throw std::runtime_error("Component not registered - call registerComponent() beforehand");

        // Add to ComponentRegistry
        std::string name = typeid(T).name();
        if (m_registry.at(name).at(entity.m_id) == nullptr)
        {
            m_registry.at(name).at(entity.m_id) = std::make_shared<T>(std::forward<Args>(args)...);
            m_registry.at(name).at(entity.m_id)->m_world = this;
            m_registry.at(name).at(entity.m_id)->m_entity = entity.m_id;

            #ifdef DIVVY_DEBUG
            std::cout << "-- Added Component " << name << " to Entity #" << entity << std::endl;
            #endif
        }
        #ifdef DIVVY_DEBUG
        else
        {
            std::cerr << "-- WARNING: Component " << name << " already present on Entity #" << entity << std::endl;
        }
        #endif

        return *std::dynamic_pointer_cast<T>(m_registry.at(name).at(entity.m_id));
    }

    /**
     * Retreive an Entity's Component
     * @returns reference to the Component
     * @throws if not found
     */
    template <class T>
    T& getComponent(const Entity& entity)
    {
        std::string name = typeid(T).name();
        if (!hasEntity(entity))
            throw std::runtime_error("Entity non-existent - call hasEntity() beforehand");
        if (!hasComponent<T>(entity))
            throw std::runtime_error("Component non-existent - call hasComponent() beforehand");
        return *std::dynamic_pointer_cast<T>(m_registry.at(typeid(T).name()).at(entity.m_id));
    }

    /**
     * Removes a Component from an Entity.
     */
    template <class T>
    void removeComponent(const Entity& entity)
    {
        std::string name = typeid(T).name();

        #ifdef DIVVY_DEBUG
        if (m_registry.at(name).at(entity.m_id) == nullptr)
            std::cerr << "-- WARNING: Component " << name << " already absent on Entity #" << entity << std::endl;
        #endif

        m_registry.at(name).at(entity.m_id) = nullptr;
    }

    /**
     * Remove an Entity from the World.
     */
    void removeEntity(Entity& entity)
    {
        // Is the entity non-existent?
        if (!hasEntity(entity))
        {
            #ifdef DIVVY_DEBUG
            std::cerr << "-- WARNING: Entity #" << entity << " is already non-existent" << std::endl;
            #endif

            return;
        }

        // Remove from ComponentRegisry
        for (auto it = m_registry.begin(); it != m_registry.end(); it++)
            it->second.at(entity.m_id) = nullptr;

        // Is top entity?
        if (entity.m_id == m_slots - 1)
            m_slots--;
        else
            m_open.insert(entity.m_id);

        #ifdef DIVVY_DEBUG
        std::cerr << "-- Removed Entity #" << entity << std::endl;
        #endif

        entity.m_id = 0;
        entity.m_world = nullptr;
    }

};

// =====================================[ Entity ]=======================================

Entity::Entity(World& world)
: m_world(&world),
  m_id(world.addEntity())
{
}

Entity::Entity(const Entity& other)
: m_world(other.m_world)
{
    if (m_world != nullptr)
        throw std::runtime_error("Null entity, cannot copy");

    m_id = m_world->addEntity(*other.m_world, other);
}

Entity::Entity(World& world, const Entity& other)
: m_world(&world),
  m_id(world.addEntity(world, other))
{
}

Entity::~Entity()
{
    if (m_world != nullptr)
        destroy();
}

Entity& Entity::operator=(const Entity& rhs)
{
    m_world = rhs.m_world;
    m_id = rhs.m_id;
    return *this;
}

template <class T, class ... Args>
inline T& Entity::add(Args&& ... args)
{
    if (m_world == nullptr)
        throw std::runtime_error("Null entity, cannot add Component");

    return m_world->addComponent<T>(*this, std::forward<Args>(args)...);
}

template <class T>
inline void Entity::remove()
{
    if (m_world == nullptr)
        throw std::runtime_error("Null entity, cannot remove Component");

    m_world->removeComponent<T>(*this);
}

template <class T>
inline T& Entity::get()
{
    if (m_world == nullptr)
        throw std::runtime_error("Null entity, cannot get Component");

    return m_world->getComponent<T>(*this);
}

inline void Entity::reset(World& world)
{
    if (m_world != nullptr)
        destroy();

    m_world = &world;
    m_id = m_world->addEntity();
}

inline void Entity::reset(const Entity& other)
{
    if (m_world != nullptr)
        destroy();

    if (other.m_world == nullptr)
        throw std::runtime_error("Null entity, cannot copy");

    m_world = other.m_world;
    m_id = m_world->addEntity(*other.m_world, other);
}

inline void Entity::reset(World& world, const Entity& other)
{
    if (m_world != nullptr)
        destroy();
    m_world = &world;
    m_id = m_world->addEntity(*m_world, other);
}

void Entity::destroy()
{
    if (m_world == nullptr)
        throw std::runtime_error("Null entity, cannot destroy");

    m_world->removeEntity(*this);
}

} // namespace divvy

#endif // DIVVY_HPP
