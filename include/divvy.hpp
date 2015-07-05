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

#include <bitset>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <typeindex>
#include <type_traits>
#include <vector>

namespace divvy {

// ===================================[ Component ]======================================

class World; // Forward declaration

/// An Entity represents a simple (unsigned) number
typedef size_t EntityID;

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

    /// Clone an existing Component to make this Component identical.
    virtual void clone(const Component& other) = 0;

    /// Provide functionality to an Entity.
    virtual void update() = 0;

protected:
    friend class World;

    /// The World that is using the Component.
    World* m_world;

    /// The EntityID that is assigned to the Component.
    EntityID m_entity;
};

/// Defines a valid Component type
template <class T>
using is_valid_component = std::enable_if_t<std::is_base_of<Component, T>::value &&
                                            !std::is_abstract<T>::value &&
                                            std::is_default_constructible<T>::value>;

// =====================================[ Entity ]=======================================

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
     * Creates a clone of an Entity in the same World
     * @param other     The Entity to clone
     */
    Entity(const Entity& other);

    /**
     * Creates a clone of an Entity in the specified World
     * @param other     The Entity to clone
     * @param world     The World to clone the Entity in.
     *                  If nullptr, clone in the same world as other
     */
    Entity(const Entity& other, World& world);

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
    template <class T, class ... Args, typename = is_valid_component<T>>
    inline T& add(Args&& ... args);

    /**
     * Check if a Component is assined to this Entity
     * @returns true if component is assigned, false otherwise
     */
    template <class T, typename = is_valid_component<T>>
    inline bool has();

    /**
     * Retrieve a Component
     * @returns reference to the Component
     */
    template <class T, typename = is_valid_component<T>>
    inline T& get();

    /**
     * Removes a Component from this Entity
     */
    template <class T, typename = is_valid_component<T>>
    inline void remove();

    /**
     * Recreates an uninitialized Entity
     */
    inline void reset();

    /**
     * Recreates an Entity in the specified World
     * @param world     The world used to create an Entity
     */
    inline void reset(World& world);

    /**
     * Recreates a clone of an Entity in the same World
     * @param other     The Entity to clone
     */
    inline void reset(const Entity& other);

    /**
     * Recreates a clone of an Entity in the specified World
     * @param other     The Entity to clone
     * @param world     The World to clone the Entity in.
     *                  If nullptr, clone in the same world as other
     */
    inline void reset(const Entity& other, World& world);

    /**
     * Checks whether an Entity is valid
     * @returns true if valid, false otherwise
     */
    inline bool valid()
    {
        return (m_world == nullptr ? false : true);
    }

    /**
     * Remove an Entity from the World.
     */
    inline void destroy();

private:
    friend class World;
    friend std::ostream& operator<<(std::ostream& stream, const Entity& entity);

    /// The World in which this Entity exists in.
    World* m_world = nullptr;

    /// Identification number to reference the Entity.
    EntityID m_id = 0;
};

/**
 * Displays the EntityID of an Entity to a standard output stream.
 * @param   stream      The stream to ouput to.
 * @param   entity      The Entity to display information of
 * @returns The stream for further usage.
 */
std::ostream& operator<<(std::ostream& stream, const Entity& entity)
{
    stream << "Entity #" << entity.m_id;
    return stream;
}

// =================================[ ComponentPool ]====================================

/**
 * Base polymorphic Component container.
 */
class ComponentPool
{
public:
    /**
     * Destructor.
     */
    virtual ~ComponentPool() {}

    /**
     * Add a Component to an Entity
     */
    virtual Component& add(int index) = 0;

    /**
     * Access a Component at the specified index.
     */
    virtual Component& at(size_t index) = 0;

    /**
     * Access a constant Component at the specified index.
     */
    virtual const Component& at(size_t index) const = 0;

    /**
     * Check if an Entity has a Component
     * @returns true if the Entity has the Component, false otherwise
     */
    virtual bool has(size_t index) = 0;

    /**
     * Remove a Component from an Entity
     */
    virtual void remove(size_t index) = 0;

    /**
     * Resize the pool to allow for more Components
     */
    virtual void resize(size_t size) = 0;

    /**
     * Updates all the Components in the pool.
     */
    virtual void update() = 0;
};

// ================================[ ComponentSegment ]==================================

/**
 * Derived polymorphic Component container. This is where all of the user-created
 * Components will be held and referenced.
 */
template <class T>
class ComponentSegment : public ComponentPool
{
private:
    /// Collection of the specified derived Component
    std::vector<T> m_segment;

    /// Record of the active Components
    std::vector<bool> m_active;

public:
    /**
     * Destructor.
     */
    virtual ~ComponentSegment() {}

    /**
     * Adds a derived Component to an Entity
     */
    virtual Component& add(int index)
    {
        if (index >= m_segment.size())
            throw std::runtime_error("ComponentSegment index out of bounds");

        m_active.at(index) = true;

        return at(index);
    }

    /**
     * Access a Component at the specified index.
     */
    virtual Component& at(size_t index)
    {
        return static_cast<Component&>(m_segment.at(index));
    }

    /**
     * Access a constant Component at the specified index.
     */
    virtual const Component& at(size_t index) const
    {
        return static_cast<const Component&>(m_segment.at(index));
    }

    /**
     * Check if an Entity has the specified Component
     * @returns true if the Entity has the Component, false otherwise
     */
    virtual bool has(size_t index)
    {
        try
        {
            return m_active.at(index);
        }
        catch (std::out_of_range e)
        {
            return false;
        }
    }

    /**
     * Removes a derived Component from an Entity
     */
    virtual void remove(size_t index)
    {
        try
        {
            m_active.at(index) = false;
        }
        catch (std::out_of_range e)
        {
            throw std::runtime_error("Cannot remove, Entity out of bounds");
        }
    }

    /**
     * Resize the pool to allow for more Components
     */
    virtual void resize(size_t size)
    {
        try
        {
            m_segment.resize(size);
            m_active.resize(size, false);
        }
        catch (std::bad_alloc e)
        {
            throw std::runtime_error("Failed to resize ComponentPool");
        }
    }

    /**
     * Updates all the active Components
     */
    virtual void update()
    {
        for (int i = 0; i < m_segment.size(); i++)
        {
            if (m_active.at(i))
                m_segment.at(i).update();
        }
    }
};

// =====================================[ World ]========================================

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
    typedef std::unique_ptr<ComponentPool> Pool;
    typedef std::map<std::type_index, Pool> ComponentRegistry;

    /// The local registry of Components types and the Entites that use them.
    ComponentRegistry m_registry;

    /// Collection of Entities created in this World
    std::vector<std::reference_wrapper<Entity>> m_entities;

    /// Current capacity of possible Entities that could exist in the World.
    size_t m_capacity = 0;

    /// Count of Entities currently existing in the World.
    size_t m_count = 0;

    /// An ordered queue in which Entities were deleted in.
    /// Serves the purpose of filling in gaps in memory where Entities were
    /// previously deleted.
    std::set<int> m_open;

private:
    /**
     * Checks whether an Entity is nonexistent or null.
     * @returns true if existing, false otherwise
     */
    inline bool hasEntity(const Entity& entity)
    {
        if (m_open.find(entity.m_id) != m_open.end() || m_capacity <= entity.m_id)
            return false;
        return true;
    }

    /**
     * Checks whether an Entity contains a specific Component.
     * @returns true if Entity contains the specific Component, false otherwise
     */
    template <class T, typename = is_valid_component<T>>
    inline bool hasComponent(const Entity& entity)
    {
        if (m_registry.find(typeid(T)) == m_registry.end() ||
           !m_registry.at(typeid(T))->has(entity.m_id))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

public:
    /**
     * Register a Component to this World.
     */
    template <class T, typename = is_valid_component<T>>
    void add()
    {
        m_registry.insert(std::make_pair(std::type_index(typeid(T)), std::make_unique<ComponentSegment<T>>()));

        #ifdef DIVVY_DEBUG
        std::cout << "-- Registered Component Type: " << typeid(T).name() << std::endl;
        #endif
    }

    /**
     * Checks whether a Component is registered in the World.
     * @returns true if the Component is registered, false otherwise
     */
    template <class T, typename = is_valid_component<T>>
    inline bool has()
    {
        if (m_registry.find(typeid(T)) == m_registry.end())
            return false;
        return true;
    }

    /**
     * Unregister a Component in this World.
     */
    template <class T, typename = is_valid_component<T>>
    void remove()
    {
        m_registry.erase(typeid(T));

        #ifdef DIVVY_DEBUG
        std::cout << "-- Registered Component Type: " << typeid(T).name() << std::endl;
        #endif
    }

    /**
     * Clears the World of all Entities and Components
     */
    void clear()
    {
        // Uninitialize every Entity in this World
        for (Entity& entity : m_entities)
        {
            entity.m_id = 0;
s            entity.m_world = nullptr;
        }

        // Unregister all Components
        for (auto it = m_registry.begin(); it != m_registry.end(); it++)
            m_registry.erase(it);
    }

    /**
     * Update all the Components registered in the World
     */
    void update()
    {
        // Update all Components
        for (auto it = m_registry.begin(); it != m_registry.end(); it++)
            for (int i = 0; i < m_capacity; i++)
                if (it->second->has(i) == true && m_open.find(i) == m_open.end())
                    it->second->at(i).update();
    }

    /**
     * Destructor.
     * Invalidates all Entities that is assigned to this World
     */
    ~World()
    {
        // Uninitialize every Entity in this World
        for (Entity& entity : m_entities)
        {
            entity.m_id = 0;
            entity.m_world = nullptr;
        }
    }

private:
    friend class Entity;
    friend class Component;

    /**
     * Create an Entity in the World.
     * @returns newly created Entity
     */
    EntityID addEntity(Entity& entity)
    {
        int index;

        if (m_open.size() != 0) // Free m_capacity available - reuse slots
        {
            index = *(m_open.begin());      // Set index to open slot
            m_open.erase(m_open.begin());   // Remove open slots
            m_entities.at(index) = entity;  // Add to Entity collection

            #ifdef DIVVY_DEBUG
            std::cout << "-- Added new Entity at #" << index << " (reused)" << std::endl;
            #endif
        }
        else // No open slots available - allocate more slots
        {
            m_capacity++; // Increase capacity

            // Resize ComponentRegistry
            for (auto it = m_registry.begin(); it != m_registry.end(); it++)
                it->second->resize(m_capacity);

            index = m_capacity - 1;         // Set index to newly allocated slot
            m_entities.push_back(entity);   // Push to Entity collection

            #ifdef DIVVY_DEBUG
            std::cout << "-- Added new Entity at #" << index << std::endl;
            #endif
        }

        m_count++; // Increase Entity count

        return index;
    }

    /**
     * Clone an Entity
     * @returns cloned Entity
     */
    EntityID addEntity(Entity& entity, const Entity& other)
    {
        EntityID id = addEntity(entity);

        // Are the Worlds the same?
        if (other.m_world == this)
        {
            // Clone all Components
            for (auto it = m_registry.begin(); it != m_registry.end(); it++)
            {
                if (it->second->has(other.m_id))
                {
                    m_registry.at(it->first)->add(id);
                    m_registry.at(it->first)->at(id).clone(it->second->at(other.m_id));
                    m_registry.at(it->first)->at(id).m_world = this;
                    m_registry.at(it->first)->at(id).m_entity = id;
                }
            }
        }
        else // Worlds are different
        {
            // Clone over related Components
            for (auto it = other.m_world->m_registry.begin(); it != other.m_world->m_registry.end(); it++)
            {
                if (it->second->has(other.m_id) &&                  // For every Component
                    m_registry.find(it->first) != m_registry.end()) // also registered in this World
                {
                    m_registry.at(it->first)->add(id);
                    m_registry.at(it->first)->at(id).clone(it->second->at(other.m_id));
                    m_registry.at(it->first)->at(id).m_world = this;
                    m_registry.at(it->first)->at(id).m_entity = id;
                }
            }
        }

        return id;
    }

    /**
     * Remove an Entity from the World.
     */
    void removeEntity(Entity& entity)
    {
        // Does the entity exist?
        if (hasEntity(entity))
        {
            // Remove from ComponentRegisry
            for (auto it = m_registry.begin(); it != m_registry.end(); it++)
                it->second->remove(entity.m_id);

            // Remove from Entity collection
            m_entities.erase(m_entities.begin() + entity.m_id);

            // Is top entity?
            if (entity.m_id == m_capacity - 1)
                m_capacity--;
            else
                m_open.insert(entity.m_id);

            // Set Entity to uninitialized
            entity.m_id = 0;
            entity.m_world = nullptr;

            // Size decrease
            m_count--;

            #ifdef DIVVY_DEBUG
            std::cerr << "-- Removed " << entity << std::endl;
            #endif
        }
        #ifdef DIVVY_DEBUG
        else
            std::cerr << "-- WARNING: " << entity << " is already non-existent" << std::endl;
        #endif
    }

    /**
     * Assign a Component to an Entity
     * @throws
     * @returns reference to the Component assigned
     */
    template <class T, class ... Args, typename = is_valid_component<T>>
    T& addComponent(const Entity& entity, Args&& ... args)
    {
        if (!hasEntity(entity))
            throw std::runtime_error("Entity non-existent - call Entity.reset() beforehand");

        if (!has<T>())
            throw std::runtime_error("Component not registered - call World.add<T>() beforehand");

        auto& type = typeid(T);

        // Add to ComponentRegistry
        if (!m_registry.at(type)->has(entity.m_id))
        {
            m_registry.at(type)->add(entity.m_id);
            m_registry.at(type)->at(entity.m_id).clone(T(std::forward<Args>(args)...));
            m_registry.at(type)->at(entity.m_id).m_world = this;
            m_registry.at(type)->at(entity.m_id).m_entity = entity.m_id;

            #ifdef DIVVY_DEBUG
            std::cout << "-- Added Component " << type.name() << " to " << entity << std::endl;
            #endif
        }
        #ifdef DIVVY_DEBUG
        else
        {
            std::cerr << "-- WARNING: Component " << type.name() << " already present on " << entity << std::endl;
        }
        #endif

        return static_cast<T&>(m_registry.at(type)->at(entity.m_id));
    }

    /**
     * Retreive an Entity's Component
     * @returns reference to the Component
     * @throws if not found
     */
    template <class T, typename = is_valid_component<T>>
    T& getComponent(const Entity& entity)
    {
        if (!hasEntity(entity))
            throw std::runtime_error("Entity non-existent - call hasEntity() beforehand");

        if (!hasComponent<T>(entity))
            throw std::runtime_error("Component non-existent - call hasComponent() beforehand");

        return static_cast<T&>(m_registry.at(typeid(T))->at(entity.m_id));
    }

    /**
     * Removes a Component from an Entity.
     */
    template <class T, typename = is_valid_component<T>>
    void removeComponent(const Entity& entity)
    {
        auto& type = typeid(T);

        #ifdef DIVVY_DEBUG
        if (!m_registry.at(type)->has(entity.m_id))
            std::cerr << "-- WARNING: Component " << type.name() << " already absent on " << entity << std::endl;
        #endif

        m_registry.at(type)->remove(entity.m_id);
    }

};

// =====================================[ Entity ]=======================================

Entity::Entity(World& world)
: m_world(&world),
  m_id(world.addEntity(*this))
{
}

Entity::Entity(const Entity& other)
: m_world(other.m_world)
{
    if (valid())
    {
        m_id = m_world->addEntity(*this, other);
    }
    #ifdef DIVVY_DEBUG
    else
    {
        std::cerr << "-- WARNING: Copying an uninitialized Entity" << std::endl;
    }
    #endif
}

Entity::Entity(const Entity& other, World& world)
: m_world(&world),
  m_id(world.addEntity(*this, other))
{
}

Entity::~Entity()
{
    reset();
}

Entity& Entity::operator=(const Entity& rhs)
{
    m_world = rhs.m_world;
    m_id = rhs.m_id;
    return *this;
}

template <class T, class ... Args, typename>
inline T& Entity::add(Args&& ... args)
{
    if (!valid())
        throw std::runtime_error("Uninitialized Entity, cannot add Component");

    return m_world->addComponent<T>(*this, std::forward<Args>(args)...);
}

template <class T, typename>
inline bool Entity::has()
{
    if (!valid())
        throw std::runtime_error("Uninitialized Entity, cannot check for Component");

    return m_world->hasComponent<T>(*this);
}

template <class T, typename>
inline T& Entity::get()
{
    if (!valid())
        throw std::runtime_error("Uninitialized Entity, cannot get Component");

    return m_world->getComponent<T>(*this);
}

template <class T, typename>
inline void Entity::remove()
{
    if (!valid())
        throw std::runtime_error("Uninitialized Entity, cannot remove Component");

    m_world->removeComponent<T>(*this);
}

inline void Entity::reset()
{
    if (valid())
        m_world->removeEntity(*this);
}

inline void Entity::reset(World& world)
{
    if (valid())
        m_world->removeEntity(*this);

    m_world = &world;
    m_id = world.addEntity(*this);
}

inline void Entity::reset(const Entity& other)
{
    if (valid())
        m_world->removeEntity(*this);

    m_world = other.m_world;

    if (valid())
    {
        m_id = m_world->addEntity(*this, other);
    }
    #ifdef DIVVY_DEBUG
    else
    {
        std::cerr << "-- WARNING: Resetting to a copy of an uninitialized Entity" << std::endl;
    }
    #endif
}

inline void Entity::reset(const Entity& other, World& world)
{
    if (valid())
        m_world->removeEntity(*this);

    m_world = &world;
    m_id = world.addEntity(*this, other);
}

} // namespace divvy

#endif // DIVVY_HPP
