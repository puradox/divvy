/*
 * Divvy v0.7 - a modern, simple component framework.
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

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeindex>
#include <type_traits>
#include <vector>

namespace divvy {

// ===================================[ Component ]======================================

class Entity; // Forward declaration

/**
 * Component is a base class that provides functionality for Entities
 * through the update function. Components can also be used to store
 * information about entites.
 */
class Component
{
public:
    /**
     * Allow derived classes to have a destructor.
     */
    virtual ~Component() {}

    /**
     * Clone an existing Component to make this Component identical.
     *
     * @param other     The other Component to create a clone of.
     */
    virtual void clone(const Component& other) = 0;

    /**
     * Provide functionality to an Entity.
     */
    virtual void update() = 0;

protected:
    /// The Entity that is assigned to this Component.
    Entity* m_entity = nullptr;

    friend class World;
};

/**
 * Defines a valid Component type.
 *
 * If you are getting a compiler error here, you haven't made a valid Component.
 *
 * A valid component has the following characteristics:
 *    - Publicly inherits the Component class
 *    - Contains no pure virtual methods
 *    - Has a default constructor (takes no arguments)
 *    - Defined before usage ~ not simply forward declared (this is a compile-time check)
 */
template <class T>
using is_valid_component = typename std::enable_if<std::is_base_of<Component,T>::value &&
                                                  !std::is_abstract<T>::value &&
                                                   std::is_default_constructible<T>::value
                                                   >::type;

/**
 * Shorter way to convert Component types when implementing the virtual clone method.
 *
 * @param other     The Component to convert.
 *
 * @returns         Constant reference to the derived Component.
 */
template <class T>
inline const T& cast(const Component& other)
{
    return static_cast<const T&>(other);
}


// =====================================[ Entity ]=======================================

class World; // Forward declaration

/// An Entity identification represents a simple (unsigned) number.
typedef size_t EntityID;

/**
 * Entity is an indentifier that unites a collection of Components of a specified World.
 * It also acts as the interface that allows addition, removal, and retrieval of Components.
 * Along with these functionalities, Entity is also copyable and movable.
 */
class Entity
{
public:
    /**
     * Creates an invalid Entity.
     */
    Entity() {}

    /**
     * Creates an Entity in the specified World.
     *
     * @param world     The world used to create an Entity.
     */
    Entity(World& world);

    /**
     * Moves an Entity into this Entity.
     *
     * @param other     The Entity to move.
     */
    Entity(Entity&& other);

    /**
     * Creates a clone of an Entity in the same World.
     *
     * @param other     The Entity to clone.
     */
    Entity(const Entity& other);

    /**
     * Creates a clone of an Entity in the specified World.
     *
     * @param other     The Entity to clone.
     * @param world     The World to clone the Entity in.
     */
    Entity(const Entity& other, World& world);

    /**
     * Destructor
     */
    ~Entity();

    /**
     * Assign a Component to this Entity.
     *
     * @return          A reference to the Component assigned.
     */
    template <class T, class ... Args, typename = is_valid_component<T>>
    inline T& add(Args&& ... args);

    /**
     * Check if a Component is assigned to this Entity.
     *
     * @return          True if component is assigned, false otherwise.
     */
    template <class T, typename = is_valid_component<T>>
    inline bool has();

    /**
     * Retrieve a Component
     *
     * @return          Reference to the Component.
     */
    template <class T, typename = is_valid_component<T>>
    inline T& get();

    /**
     * Remove a Component from this Entity.
     */
    template <class T, typename = is_valid_component<T>>
    inline void remove();

    /**
     * Recreate an unvalid Entity.
     */
    inline void reset();

    /**
     * Recreate an Entity in the specified World.
     *
     * @param world     The world used to create an Entity.
     */
    inline void reset(World& world);

    /**
     * Move an Entity in the same World.
     *
     * @param other     The Entity to move.
     */
    inline void reset(Entity&& other);

    /**
     * Recreate a clone of an Entity in the same World.
     *
     * @param other     The Entity to clone.
     */
    inline void reset(const Entity& other);

    /**
     * Recreate a clone of an Entity in the specified World.
     *
     * @param other     The Entity to clone.
     * @param world     The World to clone the Entity in.
     */
    inline void reset(const Entity& other, World& world);

    /**
     * Check whether an Entity is valid.
     *
     * @return true if valid, false otherwise.
     */
    inline bool valid()
    {
        return (m_world == nullptr ? false : true);
    }

private:
    /// The World in which this Entity exists in.
    World* m_world = nullptr;

    /// Identification number to reference the Entity.
    EntityID m_id = 0;

    friend class World;
    friend std::ostream& operator<<(std::ostream& stream, const Entity& entity);
};

/**
 * Displays the EntityID of an Entity to a standard output stream.
 *
 * @param   stream      The stream to ouput to.
 * @param   entity      The Entity to display information about.
 *
 * @return              The stream for further usage.
 */
std::ostream& operator<<(std::ostream& stream, const Entity& entity)
{
    stream << "Entity #" << entity.m_id;
    return stream;
}

// =================================[ ComponentPool ]====================================

/**
 * Base polymorphic Component container.
 *
 * Based on article "Fast polymorphic collections" by Joaquín M López Muñoz.
 * (http://bannalia.blogspot.com/2014/05/fast-polymorphic-collections.html)
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
     *
     * @param index     The EntityID of the Entity.
     *
     * @return          Reference to the newly added Component.
     */
    virtual Component& add(size_t index) = 0;

    /**
     * Access a Component at the specified index.
     *
     * @param index     The EntityID of the Entity.
     *
     * @return          Reference to the Component at the index location.
     */
    virtual Component& at(size_t index) = 0;

    /**
     * Access a constant Component at the specified index.
     *
     * @param index     The EntityID of the Entity.
     *
     * @return          Reference to the Component at the index location.
     */
    virtual const Component& at(size_t index) const = 0;

    /**
     * Check if an Entity has a Component
     *
     * @param index     The EntityID of the Entity.
     *
     * @return          True if the Entity has the Component, false otherwise.
     */
    virtual bool has(size_t index) = 0;

    /**
     * Remove a Component from an Entity
     *
     * @param index     The EntityID of the Entity.
     */
    virtual void remove(size_t index) = 0;

    /**
     * Resize the pool to allow for more Components
     *
     * @param size      The desired size of elements in the pool.
     */
    virtual void resize(size_t size) = 0;

    /**
     * Updates all active Components in the pool.
     */
    virtual void update() = 0;
};

// ================================[ ComponentSegment ]==================================

/**
 * Derived polymorphic Component container.
 * This is where all of the user-created Components will be held and referenced.
 *
 * Based on article "Fast polymorphic collections" by Joaquín M López Muñoz.
 * (http://bannalia.blogspot.com/2014/05/fast-polymorphic-collections.html)
 */
template <class T>
class ComponentSegment : public ComponentPool
{
public:
    /**
     * Destructor.
     */
    virtual ~ComponentSegment() {}

    /**
     * Add a Component to an Entity
     *
     * @param index     The EntityID of the Entity.
     *
     * @return          Reference to the newly added Component.
     */
    virtual Component& add(size_t index)
    {
        if (index >= m_segment.size())
            throw std::runtime_error("ComponentSegment index out of bounds");

        m_active.at(index) = true;

        return at(index);
    }

    /**
     * Access a Component at the specified index.
     *
     * @param index     The EntityID of the Entity.
     *
     * @return          Reference to the Component at the index location.
     */
    virtual Component& at(size_t index)
    {
        return static_cast<Component&>(m_segment.at(index));
    }

    /**
     * Access a constant Component at the specified index.
     *
     * @param index     The EntityID of the Entity.
     *
     * @return          Reference to the Component at the index location.
     */
    virtual const Component& at(size_t index) const
    {
        return static_cast<const Component&>(m_segment.at(index));
    }

    /**
     * Check if an Entity has a Component
     *
     * @param index     The EntityID of the Entity.
     *
     * @return          True if the Entity has the Component, false otherwise.
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
     * Remove a Component from an Entity
     *
     * @param index     The EntityID of the Entity.
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
     *
     * @param size      The desired size of elements in the pool.
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
     * Updates all active Components in the pool.
     */
    virtual void update()
    {
        for (unsigned int i = 0; i < m_segment.size(); i++)
        {
            if (m_active.at(i))
                m_segment.at(i).update();
        }
    }

private:
    /// Collection of the specified derived Component
    std::vector<T> m_segment;

    /// Record of the active Components
    std::vector<bool> m_active;
};

// =====================================[ World ]========================================

/**
 * Implement make_unique for C++11 ~ since it was "partly an oversight."
 *
 * @param args      Arguments to forward to the creation of <T> object.
 *
 * @return          A std::unique_ptr<T> to the newly created object.
 */
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args )
{
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}

/**
 * World is the heart of all Component operations, as it calls each Component's
 * update method. The creation of Entities and Components happen within a
 * world. Since Entities and Components are localized within a World, there
 * is the possibility to have multiple Worlds, each being different than the
 * other.
 */
class World
{
public:
    /**
     * Destructor; invalidates all Entities that is assigned to this World.
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

    /**
     * Register a Component type to this World.
     */
    template <class T, typename = is_valid_component<T>>
    void add()
    {
        m_registry.insert(std::make_pair(std::type_index(typeid(T)), make_unique<ComponentSegment<T>>()));

        #ifdef DIVVY_DEBUG
        std::cout << "-- Registered Component Type: " << typeid(T).name() << std::endl;
        #endif
    }

    /**
     * Check whether a Component type is registered in the World.
     *
     * @return          True if the Component is registered, false otherwise.
     */
    template <class T, typename = is_valid_component<T>>
    inline bool has()
    {
        if (m_registry.find(typeid(T)) == m_registry.end())
            return false;
        return true;
    }

    /**
     * Unregister a Component type in this World.
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
     * Clear the World of all Entities and Components.
     */
    void clear()
    {
        // Uninitialize every Entity in this World
        for (Entity& entity : m_entities)
        {
            entity.m_id = 0;
            entity.m_world = nullptr;
        }

        // Unregister all Components
        for (auto it = m_registry.begin(); it != m_registry.end(); it++)
            m_registry.erase(it);
    }

    /**
     * Update all the Components in this World.
     */
    void update()
    {
        for (auto it = m_registry.begin(); it != m_registry.end(); it++)          // For every Component type
            for (unsigned int i = 0; i < m_capacity; i++)                         // In the capacity range
                if (it->second->has(i) == true && m_open.find(i) == m_open.end()) // That is active and existing
                    it->second->at(i).update();                                   // Update.
    }

private:
    /**
     * Checks whether an Entity is nonexistent or null.
     *
     * @param entity    Reference to the Entity to check.
     *
     * @return          True if existing, false otherwise.
     */
    inline bool hasEntity(const Entity& entity)
    {
        if (m_open.find(entity.m_id) != m_open.end() || m_capacity <= entity.m_id)
            return false;
        return true;
    }

    /**
     * Checks whether an Entity contains a specific Component.
     *
     * @param entity    Reference to the Entity to check.
     *
     * @return          True if Entity contains the specific Component, false otherwise.
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

    /**
     * Create an Entity in the World.
     *
     * @param entity    Reference to the Entity that wants to be added.
     *
     * @return          Newly created EntityID.
     */
    EntityID addEntity(Entity& entity)
    {
        int index;

        if (m_open.size() != 0)             // Free m_capacity available - reuse slots
        {
            index = *(m_open.begin());      // Set index to open slot
            m_open.erase(m_open.begin());   // Remove open slots
            m_entities.at(index) = entity;  // Add to Entity collection

            #ifdef DIVVY_DEBUG
            std::cout << "-- Added new Entity at #" << index << " (reused)" << std::endl;
            #endif
        }
        else                                // No open slots available - allocate more slots
        {
            m_capacity++;                   // Increase capacity

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
     *
     * @param entity    Reference to the Entity that wants to be added.
     * @param other     Reference to the Entity that acts as a template for Components.
     *
     * @return          Newly created EntityID.
     */
    EntityID addEntity(Entity& entity, const Entity& other)
    {
        EntityID id = addEntity(entity);

        if (other.m_world == this)      // Are the Worlds the same?
        {
            // Clone all Components
            for (auto it = m_registry.begin(); it != m_registry.end(); it++)
            {
                if (it->second->has(other.m_id))
                {
                    m_registry.at(it->first)->add(id);
                    m_registry.at(it->first)->at(id).clone(it->second->at(other.m_id));
                    m_registry.at(it->first)->at(id).m_entity = &entity;
                }
            }
        }
        else                            // Worlds are different
        {
            // Clone over related Components
            for (auto it = other.m_world->m_registry.begin(); it != other.m_world->m_registry.end(); it++)
            {
                if (it->second->has(other.m_id) &&                  // For every Component
                    m_registry.find(it->first) != m_registry.end()) // also registered in this World
                {
                    m_registry.at(it->first)->add(id);
                    m_registry.at(it->first)->at(id).clone(it->second->at(other.m_id));
                    m_registry.at(it->first)->at(id).m_entity = &entity;
                }
            }
        }

        return id;
    }

    /**
     * Remove an Entity from the World.
     *
     * @param entity    Reference to the Entity that wants to be removed.
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
                m_capacity--;                   // Decrease capacity
            else
                m_open.insert(entity.m_id);     // Insert open slot

            // Set Entity to uninitialized
            entity.m_id = 0;
            entity.m_world = nullptr;

            m_count--; // Size decrease

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
     * Replace an Entity with another Entity.
     *
     * @param entity    Reference to the Entity that wants to exist.
     * @param other     Reference to the Entity that will be replaced.
     */
    void replaceEntity(Entity& entity, Entity& other)
    {
        m_entities.at(entity.m_id) = other;
    }

    /**
     * Assign a Component to an Entity.
     *
     * @param entity    Reference to the target Entity.
     * @param args      Arguments to feed to the Component's constructor.
     *
     * @return          Reference to the Component assigned.
     */
    template <class T, class ... Args, typename = is_valid_component<T>>
    T& addComponent(Entity& entity, Args&& ... args)
    {
        if (!hasEntity(entity))
            throw std::runtime_error("Entity non-existent - call Entity.reset() beforehand");

        if (!has<T>())
            throw std::runtime_error("Component not registered - call World.add<T>() beforehand");

        auto& type = typeid(T);

        // Add to ComponentRegistry if not existing
        if (!m_registry.at(type)->has(entity.m_id))
        {
            m_registry.at(type)->add(entity.m_id);
            m_registry.at(type)->at(entity.m_id).clone(T(std::forward<Args>(args)...));
            m_registry.at(type)->at(entity.m_id).m_entity = &entity;

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
     * Retreive an Entity's Component.
     *
     * @param entity    Reference to the target Entity.
     *
     * @return          Reference to the Component retrieved.
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
     *
     * @param entity    Reference to the target Entity.
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

private:
    /**
     * These are the essential typedefs that describe what a registry and pool are and
     * how to access their elements. Take note of the types used.
     */
    typedef std::unique_ptr<ComponentPool>  Pool;
    typedef std::map<std::type_index, Pool> ComponentRegistry;

    /// The local registry of Components types and the Entites that use them.
    ComponentRegistry m_registry;

    /// Collection of Entities created in this World
    std::vector<std::reference_wrapper<Entity>> m_entities;

    /**
     * An ordered queue in which Entities were deleted in.
     * Serves the purpose of filling in gaps in memory where Entities were
     * previously deleted.
     */
    std::set<int> m_open;

    /// Current capacity of possible Entities that could exist in the World.
    size_t m_capacity = 0;

    /// Count of Entities currently existing in the World.
    size_t m_count = 0;

    friend class Entity;
};

// ============================[ Entity Implementation ]=================================

/*
 * This is the remainder of the Entity implementation code.
 * Go back to the Entity class declaration if you want to view the documentation.
 *
 * I was forced to wait until after I implemented the World class to implement all this
 * because all of the Entity methods redirects to World methods.
 *
 * Remember, Entity is simply an interface.
 *
 * Without further ado...
 */

Entity::Entity(World& world)
: m_world(&world),
  m_id(world.addEntity(*this))
{
}

Entity::Entity(Entity&& other)
{
    if (other.m_world)
    {
        other.m_world->replaceEntity(other, *this);

        m_id = other.m_id;
        m_world = other.m_world;
        other.m_world = nullptr;
    }
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

inline void Entity::reset(Entity&& other)
{
    if (other.m_world)
    {
        other.m_world->replaceEntity(other, *this);

        m_id = other.m_id;
        m_world = other.m_world;
        other.m_world = nullptr;
    }
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
