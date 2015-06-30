/*
 * Divvy, a simple component entity framework.
 * Author: Sam Balana <sambalana247@gmail.com>
 *
 * Licensed under the The MIT License (MIT)
 *
 * Copyright (c) 2015 Sam Balana
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

#ifdef DIVVY_DEBUG
#include <iostream>
#endif

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace divvy {

// =============================[ Entity ]=====================================

/// An Entity represents a simple (unsigned) number, with 0 being a null entity
typedef std::uint_fast32_t Entity;

// =============================[ Component ]==================================

class World; // Forward declaration

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

    /// The World that is using the Component.
    World* world;

    /// The Entity that is assigned to the Component.
    Entity entity;
};

// =============================[ World ]======================================

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
    /// The local registry of Components types and the Entites that use them.
    ComponentRegistry registry;

    /// Number of Entities existing in the World.
    /// Slots is initially set to 1 since 0 needs to represent a null Entity.
    size_t slots = 1;

    /// An ordered queue in which Entities were deleted in.
    /// Serves the purpose of filling in gaps in memory where Entities were
    /// previously deleted.
    std::set<int> openSlots;

public:
    /**
     * Create an Entity in the World.
     * @returns newly created Entity
     */
    Entity addEntity()
    {
        if (openSlots.size() != 0) // Free slots available
        {
            // Remove the open slot
            int index = *(openSlots.begin());
            openSlots.erase(openSlots.begin());

            #ifdef DIVVY_DEBUG
            std::cout << "-- Added new Entity at #" << index << " (reused)" << std::endl;
            #endif

            return index;
        }
        else
        {
            // Resize ComponentRegistry
            for (auto it = registry.begin(); it != registry.end(); it++)
                it->second.resize(slots + 1);

            #ifdef DIVVY_DEBUG
            std::cout << "-- Added new Entity at #" << slots << std::endl;
            #endif

            // Add a slot for the new Entity
            return slots++;
        }
    }

    /**
     * Clone an Entity in the World.
     * @returns cloned Entity
     */
    Entity addEntity(Entity other)
    {
        Entity e = addEntity();

        // Clone all Components of other Entity
        for (auto it = registry.begin(); it != registry.end(); it++)
            if (it->second.at(other) != nullptr)
                it->second.at(e) = it->second.at(other)->clone();

        return e;
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
     * Registers a Component on the ComponentRegistry of the World.
     */
    template <class T>
    void registerComponent()
    {
        if (!validComponent<T>())
            throw std::runtime_error("Not a valid Component - class must derive from Component");

        // Register on the ComponentRegistry
        std::string name = typeid(T).name();
        registry.insert(std::make_pair(name, std::vector<std::shared_ptr<Component>>()));

        #ifdef DIVVY_DEBUG
        std::cout << "-- Registered Component Type: " << name << std::endl;
        #endif
    }

    /**
     * Checks whether an Entity is nonexistent or null.
     * @returns true if existing, false otherwise
     */
    inline bool hasEntity(Entity entity)
    {
        if (openSlots.find(entity) != openSlots.end() || slots <= entity || entity == 0)
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
        if (registry.find(typeid(T).name()) == registry.end())
            return false;
        return true;
    }

    /**
     * Checks whether an Entity contains a specific Component.
     * @returns true if Entity contains the specific Component, false otherwise
     */
    template <class T>
    inline bool hasComponent(Entity entity)
    {
        std::string name = typeid(T).name();
        if (registry.find(name) == registry.end() || registry.at(name).at(entity) == nullptr)
            return false;
        return true;
    }

    /**
     * Assign a Component to an Entity
     * @returns reference to the Component assigned
     */
    template <class T, class ... Args>
    T& addComponent(Entity entity, Args&& ... args)
    {
        if (!validComponent<T>())
            throw std::runtime_error("Not a valid Component - class must derive from Component");

        if (!hasEntity(entity))
            throw std::runtime_error("Entity non-existent - call hasEntity() beforehand");

        if (!hasComponent<T>())
            throw std::runtime_error("Component not registered - call registerComponent() beforehand");

        // Add to ComponentRegistry
        std::string name = typeid(T).name();
        if (registry.at(name).at(entity) == nullptr)
        {
            registry.at(name).at(entity) = std::make_shared<T>(std::forward<Args>(args)...);
            registry.at(name).at(entity)->world = this;
            registry.at(name).at(entity)->entity = entity;

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

        return *std::dynamic_pointer_cast<T>(registry.at(name).at(entity));
    }

    /**
     * Retreive an Entity's Component
     * @returns reference to the Component
     * @throws if not found
     */
    template <class T>
    T& getComponent(Entity entity)
    {
        std::string name = typeid(T).name();
        if (!hasEntity(entity))
            throw std::runtime_error("Entity non-existent - call hasEntity() beforehand");
        if (!hasComponent<T>(entity))
            throw std::runtime_error("Component non-existent - call hasComponent() beforehand");
        return *std::dynamic_pointer_cast<T>(registry.at(typeid(T).name()).at(entity));
    }

    /**
     * Removes a Component from an Entity.
     */
    template <class T>
    void removeComponent(Entity entity)
    {
        std::string name = typeid(T).name();

        #ifdef DIVVY_DEBUG
        if (registry.at(name).at(entity) == nullptr)
            std::cerr << "-- WARNING: Component " << name << " already absent on Entity #" << entity << std::endl;
        #endif

        registry.at(name).at(entity) = nullptr;
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
        for (auto it = registry.begin(); it != registry.end(); it++)
            it->second.at(entity) = nullptr;

        // Is top entity?
        if (entity == slots)
            slots--;
        else
            openSlots.insert(entity);

        entity = 0;
    }

    /**
     * Update all the Components registered in the World
     */
    void update()
    {
        // Update all Components
        for (auto it = registry.begin(); it != registry.end(); it++)
            for (int i = 1; i < slots && openSlots.find(i) == openSlots.end(); i++)
                if (it->second.at(i) != nullptr)
                    it->second.at(i)->update();
    }
};

} // namespace divvy

#endif // DIVVY_HPP
