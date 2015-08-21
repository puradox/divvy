#ifndef DIVVY_COMPONENT_HPP
#define DIVVY_COMPONENT_HPP

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
	using is_valid_component = typename std::enable_if<std::is_base_of<Component, T>::value &&
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

} // namespace Divvy

#endif // DIVVY_COMPONENT_HPP