#ifndef DIVVY_COMPONENT_POOL_HPP
#define DIVVY_COMPONENT_POOL_HPP

#include <vector>

#include "Component.hpp"

namespace divvy {

	// =================================[ BaseComponentPool ]================================

	/**
	* Base polymorphic Component container.
	* Used for storing different types of ComponentPools in a single container.
	*
	* Based on article "Fast polymorphic collections" by Joaquín M López Muñoz.
	* (http://bannalia.blogspot.com/2014/05/fast-polymorphic-collections.html)
	*/
	class BaseComponentPool
	{
	public:
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
		* Returns the reserved capacity of the pool.
		*
		* @return          Reserved capacity
		*/
		virtual size_t capacity() const = 0;

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
		* Update all active Components in the pool.
		*/
		virtual void update() = 0;
	};

	// ================================[ ComponentPool ]=====================================

	/**
	* Derived polymorphic Component container.
	* This is where all of the user-created Components will be held and referenced.
	*
	* Based on article "Fast polymorphic collections" by Joaquín M López Muñoz.
	* (http://bannalia.blogspot.com/2014/05/fast-polymorphic-collections.html)
	*/
	template <class T>
	class ComponentPool : public BaseComponentPool
	{
	public:
		virtual Component& add(size_t index)
		{
			if (index >= m_pool.size())
				throw std::runtime_error("ComponentSegment index out of bounds");

			m_active.at(index) = true;

			return at(index);
		}

		virtual Component& at(size_t index)
		{
			return static_cast<Component&>(m_pool.at(index));
		}

		virtual const Component& at(size_t index) const
		{
			return static_cast<const Component&>(m_pool.at(index));
		}

		virtual size_t capacity() const
		{
			return m_pool.size();
		}

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

		virtual void resize(size_t size)
		{
			try
			{
				m_pool.resize(size);
				m_active.resize(size, false);
			}
			catch (std::bad_alloc e)
			{
				throw std::runtime_error("Failed to resize ComponentPool");
			}
		}

		virtual void update()
		{
			for (unsigned int i = 0; i < m_pool.size(); i++)
			{
				if (m_active.at(i))
					m_pool.at(i).update();
			}
		}

	private:
		/// Collection of the specified derived Component
		std::vector<T> m_pool;

		/// Record of the active Components
		std::vector<bool> m_active;
	};

} // namespace divvy

#endif // DIVVY_COMPONENT_POOL_HPP