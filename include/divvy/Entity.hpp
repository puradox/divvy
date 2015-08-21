#ifndef DIVVY_ENTITY_HPP
#define DIVVY_ENTITY_HPP

#include <iostream>

#include "Component.hpp"

namespace divvy {

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
		* Create an invalid Entity.
		*/
		Entity() {}

		/**
		* Create an Entity in the specified World.
		*
		* @param world     The world used to create an Entity.
		*/
		Entity(World& world);

		/**
		* Move an Entity into this Entity.
		*
		* @param other     The Entity to move.
		*/
		Entity(Entity&& other);

		/**
		* Create a clone of an Entity in the same World.
		*
		* @param other     The Entity to clone.
		*/
		Entity(const Entity& other);

		/**
		* Create a clone of an Entity in the specified World.
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
		inline bool valid() const
		{
			return (m_world == nullptr ? false : true);
		}

		/**
		* Converts Entity to boolean based on whether or not it is valid.
		*
		* @returns true if valid, false otherwise.
		*/
		operator bool() const
		{
			return valid();
		}

	private:
		/// The World in which this Entity exists in.
		World* m_world = nullptr;

		/// Identification number to reference the Entity.
		EntityID m_id = 0;

		friend class World;

#ifdef DIVVY_DEBUG
		friend std::ostream& operator<<(std::ostream& stream, const Entity& entity);
#endif
	};

#ifdef DIVVY_DEBUG
	/**
	* Display the EntityID of an Entity to a standard output stream.
	*
	* @param   stream      The stream to ouput to.
	* @param   entity      The Entity to display information about.
	*
	* @return              Returns stream for further usage.
	*/
	std::ostream& operator<<(std::ostream& stream, const Entity& entity)
	{
		stream << "Entity #" << entity.m_id;
		return stream;
	}
#endif

} // namespace divvy

#endif // DIVVY_ENTITY_HPP