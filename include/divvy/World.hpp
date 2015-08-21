#ifndef DIVVY_WORLD_HPP
#define DIVVY_WORLD_HPP

#include <map>
#include <memory>
#include <set>
#include <typeindex>

#include "Component.hpp"
#include "ComponentPool.hpp"
#include "Entity.hpp"

namespace divvy{

	// =====================================[ World ]========================================

	/**
	* Implementation of make_unique for C++11 ~ since it was "partly an oversight."
	* Create a unique_ptr object.
	*
	* @param args      Arguments to forward to the creation of <T> object.
	*
	* @return          A std::unique_ptr<T> to the newly created object.
	*/
	template<typename T, typename ...Args>
	std::unique_ptr<T> make_unique(Args&& ...args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
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
		* Invalidate all Entities that is assigned to this World.
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
			m_registry.insert(std::make_pair(std::type_index(typeid(T)), make_unique<ComponentPool<T>>()));
			m_registry.at(typeid(T))->resize(m_capacity);

#ifdef DIVVY_DEBUG
			std::cout << "-- Registered Component Type: " << typeid(T).name() << std::endl;
#endif
		}

		/**
		* Run a lambda on a collection of Components
		*/
		//template <class Derived&&

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
			m_registry.clear();
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
		* Check whether an Entity is nonexistent or null.
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
		* Check whether an Entity contains a specific Component.
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
			size_t index;

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
		* Remove a Component from an Entity.
		*
		* @param entity    Reference to the target Entity.
		*/
		template <class T, typename = is_valid_component<T>>
		void removeComponent(const Entity& entity)
		{
			auto& type = typeid(T);

			try
			{
#ifdef DIVVY_DEBUG
				if (!m_registry.at(type)->has(entity.m_id))
					std::cerr << "-- WARNING: Component " << type.name() << " already absent on " << entity << std::endl;
#endif
				m_registry.at(type)->remove(entity.m_id);
			}
			catch (std::out_of_range e)
			{
				throw std::runtime_error("Component type not added to World");
			}
		}

	private:
		/**
		* These are the essential typedefs that describe what a registry and pool are and
		* how to access their elements. Take note of the types used.
		*/
		typedef std::unique_ptr<BaseComponentPool> Pool;
		typedef std::map<std::type_index, Pool>    ComponentRegistry;

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
	* since all of the Entity methods redirects to World methods.
	*
	* Entity is simply an interface for World.
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

#endif // DIVVY_WORLD_HPP