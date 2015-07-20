# Divvy - A lightweight Component Entity framework [![Build Status](https://travis-ci.org/puradox/divvy.svg?branch=master)](https://travis-ci.org/puradox/divvy)
##### Current version: v0.8

Divvy is a lightweight component entity framework made in C++11. Licensed under the MIT license and designed to be extremely easy to integrate, it's purpose is to ease development where big, monolithic class structures would normally be the answer. See all of the details, benefits, and drawbacks of the *Component pattern* [here](http://gameprogrammingpatterns.com/component.html).

## Features
  - Simple, no macros or unneccessary clutter
  - Extendable, Components can be created for any purpose and are easy to make
  - Lightweight, only three fundimental parts (`Component`, `Entity`, and `World`)
  - Easy to integrate, all of Divvy is implemented in a single header file (no linking!)
  - Fast, contiguous memory storage of Components allows for faster iterations
  - Type-safe, [`std::enable_if`](http://en.cppreference.com/w/cpp/types/enable_if) ensures that Components can't be mixed up

## Purpose

Divvy was made in mind for people that desire a simple, lightweight way to utilize the Component pattern in their program. Most other Component Entity libraries are large and requires users to download, compile, then link against their library. With the introduction of Divvy, all users have to do is copy and paste the single header file into their project and start using the *Component pattern* without additional hassle!

Divvy was originally geared towards game development; however, it can be used for other areas of development that could benefit from interchangable programming bits.

## Quick Example

```C++
#include <iostream>
#include <string>

#include "divvy.hpp"

class Nametag : public divvy::Component
{
public:
    Nametag() {}

    Nametag(const std::string& name) : m_name(name) {}

    virtual void update()
    {
        std::cout << "Hello! My name is " << m_name << ".\n";
    }

    virtual void clone(const divvy::Component& other)
    {
        m_name = divvy::cast<Nametag>(other).m_name;
    }
    
    void setName(const std::string& name) { m_name = name; }

private:
    std::string m_name;
};

int main()
{
    divvy::World world;
    world.add<Nametag>();

    divvy::Entity hero(world);
    hero.add<Nametag>("Mario");

    world.update(); // OUTPUT: Hello! My name is Mario.
    
    hero.get<Nametag>().setName("Luigi");
    
    world.update(); // OUTPUT: Hello! My name is Luigi.
}
```

#### More Examples

To view more examples, visit the `examples` folder in this repository.

## Dependencies

Divvy is dependant on the C++11 STL, so make sure to use a C++11 compliant compiler when using Divvy. Don't forget to compile your C++ program with the `-std=c++11` or equivalent flag!

## Integrating Divvy into your project

To use Divvy, simply copy over the single header file `include/divvy.hpp` of this repository into your project.

## Testing

Unit tests are ran on the [GoogleTest framework](https://code.google.com/p/googletest/); however, you don't have to download/install GoogleTest in order to run the tests, since it is a git submodule of this repository.

To clone and test this repository with GoogleTest on Linux:
```
git clone --recursive https://github.com/puradox/divvy
cd divvy && mkdir build && cd build
cmake ..
make && make test
```

For a more detailed test with additional information and time log:
```
./test/divvy_test
```

# Documentation

Divvy is based on the usage of three different classes types, each will be further explained in their own section.
  - [`Component`](#component): The base class that all Components have to derive from.
  - [`Entity`](#entity): Identifier that unifies a collection of Components. An interface to manipulate Components.
  - [`World`](#world): Container for all Components and Entity associations.

## Quick Reference

For a quick reference, here is the full list of public methods available by class.

**Entity**
  - `Entity()` - Creates an invalid Entity.
  - `Entity(World& world)` - Creates an Entity in the specified World.
  - `Entity(Entity&& other)` - Moves an Entity into this Entity.
  - `Entity(const Entity& other)` - Creates a clone of an Entity in the same World.
  - `Entity(const Entity& other, World& world)` - Creates a clone of an Entity in the specified World.
  - `Component& Entity.add<Component>(...)` - Assign a Component to this Entity.
  - `Component& Entity.get<Component>()` - Retrieve a Component.
  - `bool Entity.has<Component>()` - Check if a Component is assigned to this Entity.
  - `void Entity.remove<Component>()` - Remove a Component from this Entity.
  - `void Entity.reset()` - Recreate an invalid Entity **(There is a corresponding reset method for every constructor)**
  - `Entity.valid()` - Check whether an Entity is valid.
```
**World**
```C++
void World.add<Component>() - Register a Component type to this World.
bool World.has<Component>() - Check whether a Component type is registered in this World.
void World.remove<Component>() - Unregister a Component type in this World.
void World.clear() - Clear the World of all Entities and Components.
void World.update() - Update all the Components in this World.
```

## Component

Components are essential to decoupling code and forming a modular codebase. `Component` is meant to be inherited into your own component type. To create a valid component, we must adhere to the following rules:
  1. Publicly inherit from `divvy::Component`
  2. Implement a default constructor that takes no arguments

`Component` contains two pure virtual methods that you have to implement when creating your own component:
  - `update`: provides functionality to your `Component`
  - `clone`: provides copy semantics to your `Component`

Additionally, Components hold a pointer `m_entity` to the Entity that is assigned to them. Later on, we will see how this is useful and where you could possibily use it. (See [Checking For Components](#checking-for-components))

Note that `divvy::cast<T>(other)` is the exact same as `static_cast<const T&>(other)`. The `cast` function was added in v0.6 to help readability when implementing the virtual clone method of `Component`, there is no extra functionality behind it.

## World

A `World` contains all of the possible component types that you can add to an `Entity`. 

```C++
divvy::World world;
```

#### Adding Component Types to a World

It is important that we add all the component types that we want into the `World` before we assign any components to an `Entity`.

```C++
world.add<Nametag>();
```

#### Checking Component Types in a World

```C++
world.has<Nametag>();
```

The `has` method checks whether a specific component type exists in a `World`. It returns `true` if the component type is included in the `World`, false otherwise.

#### Updating the World

Whenever a `World` is updated, all of the Components that are active inside the `World` are updated as well.

```C++
world.update();
```

#### Removing Component Types in a World

We can remove component types in the same manner in which we added them.

```C++
world.remove<Nametag>();
```

Any Entities that have the removed `Component` assigned will have it removed.

#### Clearing a World

```C++
world.clear();
```

Clearing a `World` results in the deactivation of all the Components and Entities that are associated with it. This means that any existing Entities that operate under the cleared `World` will become invalid.

## Entity

`Entity` is the interface to add, remove, and retrieve components. To act as this interface, Entities have to be assigned to a `World`, since the `World` is what holds all of the Components. If there is no `World` assigned, the `Entity` is considered to be invalid and won't be of any use. Trying to use an invalid `Entity` will result in an exception being thrown.

To create a valid Entity, either pass a `World` to the constructor,
```C++
divvy::Entity hero(world);
```
or pass a `World` to the `reset(...)` method.
```C++
divvy::Entity hero;
hero.reset(world);
```

Although these two ways are equivalent, the reset method is most useful when creating an array of `Entity`. In which case Entities would be created using the default constructor, which doesn't assign a `World`. So to combat this, you could run the `reset` method on all of the elements in the array to assign a `World`. **Note: there is a corresponding `reset` method for every constructor of `Entity`.**

#### Checking validity of an Entity

The validity of an `Entity` depends of whether or not it is associated with a `World`. You can check the validity of an `Entity` with the `valid` method, which returns either `true` or `false`.

```C++
hero.valid();
```

#### Adding Components to an Entity

```C++
hero.add<Nametag>("Mario");
```

When Components are added to Entities, the constructor that matches the parameter list of `add` will be called. This allows for overloaded constructors to be utilized.

#### Retrieving Components from an Entity

The `Entity` interface also makes retrieving components as easy as adding them.

```C++
hero.get<Nametag>().setName("Luigi");

world.update(); // OUTPUT: Hello! My name is Luigi.
```

#### Copying/Moving Entities

Entities have the additional functionality of being copable and movable. However, it is important to remember that copying and moving are fundamentially different.
  - **Copying**: calls the `clone` method of each Component, which copies the specified variables
  - **Moving**: moves the reference of the `Entity`, leaving the other `Entity` invalid

```C++
// Copying
divvy::Entity enemy(hero);
enemy.get<Nametag>().setName("Bowser");

// Moving
divvy::Entity princess = std::move(hero);
princess.get<Nametag>().setName("Peach");

// Check if the move was successful
if (princess.valid() && !hero.valid())
{
    world.update();
}

/* OUTPUT:
Hello! My name is Peach.
Hello! My name is Bowser.
*/
```

#### Copying Entities Between Worlds

If a situation appears in which you would want to copy Entities between two different worlds, there are two different ways to approach it.

You could use the overloaded constructor,
```C++
divvy::Entity otherHero(hero, otherWorld);
```
or corresponding `reset` method.
```C++
otherHero.reset(hero, otherWorld);
```

Here is a full example using the `Nametag` component in the Quick Example.
```C++
divvy::World earth;
world.add<Nametag>();
divvy::Entity human(world);
human.add<Nametag>("astronaut");

divvy::World mars;
mars.add<Nametag>();
divvy::Entity martian(human, mars);

mars.update(); // OUTPUT: Hello! My name is astronaut.
```

**When copying entities between two different worlds, only the component types that exist in both worlds will be copied over.**

#### Checking for Components

```C++
if (enemy.has<Nametag>())
{
    std::cout << "Enemy has a name! \n";
}
```

It can serve useful to check whether an `Entity` has a specific `Component`. The `has` method allows for such a check to be possible. This is especially useful to check whether an `Entity` has the required Components before adding another `Component`.

```C++
class Physics : public divvy::Component
{
    Physics()
    {
        if (!m_entity->has<Transform>() || !m_entity->has<Mass>())
        {
            std::cout << "Physics requires Transform and Mass to already be present. \n";
            m_entity->remove<Physics>();
        }
    }
    
    ...
}
```

Again, to avoid confusion, `m_entity` is a protected pointer built into `Component` that points to it's assigned `Entity`.

#### Removing Components

```C++
enemy.remove<Nametag>();
```

This immediately deactives the Component and removes it from the `Entity` that it is assigned to.

## That's all!

If you have any more questions about Divvy and how it works, you could either 
  - [Email me](mailto:sambalana247@gmail.com)
  - View the [source code](https://github.com/puradox/divvy/blob/master/include/divvy.hpp) directly (fully documented and not too long) 
