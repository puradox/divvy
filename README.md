# Divvy - A lightweight Component framework [![Build Status](https://travis-ci.org/puradox/divvy.svg?branch=master)](https://travis-ci.org/puradox/divvy)
##### Current version: v0.5

Divvy is a lightweight component entity framework made in C++11. Licensed 
under the MIT license and designed to be extremely easy to integrate, it's 
purpose is to ease development where monolithic class structures would 
normally be the answer. See all of the details, benefits, and drawbacks of
the *Component pattern* [here](http://gameprogrammingpatterns.com/component.html).

## Features
  - Simple, no macros or unneccessary clutter
  - Extendable, Components are easy to make
  - Lightweight, no need to link libraries
  - Easy to integrate, all of Divvy is implemented in a single header file (no linking!)
  - Fast, Contiguous memory storage of Components allows for faster iterations
  - Type-safe, [`std::enable_if`](http://en.cppreference.com/w/cpp/types/enable_if) ensures that Components can't be mixed up

## Purpose

Divvy was made in mind for people that desire a simple, lightweight way to utilize the Component pattern in their program. Most other Component Entity libraries are large and requires users to download, compile, then link against their library. With the introduction of Divvy, all users have to do is copy and paste the single header file into their project and start using the *Component pattern* without additional hassle!

Divvy was originally geared towards game development; however, it can be used for other areas of development that could benefit from interchangable programming bits.

## Dependencies

Divvy is dependant on the C++11 STL, so make sure to use a C++11 compliant compiler when using Divvy. Don't forget to compile your C++ program with the `-std=c++11` or equivalent flag!

## Integrating Divvy into your project

To use Divvy, simply copy over the single header file `include/divvy.hpp` in this repository into your own project.

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
        m_name = static_cast<const Nametag&>(other).m_name;
    }

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
}
```

## Getting Started - Step by Step

Divvy is based on the usage of three different classes types, each will be further explained in their own section.
  - `Component`: The base class that all Components have to derive from.
  - `Entity`: Identifier that unifies a collection of Components. Also acts as a helper class to access Components.
  - `World`: Container for all Components and Entity associations.

#### Creating Components

Components are essential to decoupling code and forming a modular codebase. A `Component` will have two neccessary virtual methods that you have implement in your own Components, `update` and `clone`.
  - `update`: provides functionality to your `Component`
  - `clone`: provides copy semantics your `Component`

Components also hold a pointer `m_entity` to the Entity that is assigned to them. Later on, we will see how this is useful and where you could possibily use it.

To start, we will need to create our own component. To create a valid component, we have to adhere to the following rules:
  1. Publicly inherit from `divvy::Component`
  2. Implement a default constructor (takes no arguments)

Let's create a component that holds a string and prints it upon update. It will resemble the one in the Quick Example, but with some added features and comments to demonstrate the possibilities of a `Component`.
```C++
#include <iostream>
#include <string>

#include "divvy.hpp" // or where ever it is located

class Nametag : public divvy::Component
{
public:
    // Default constructor
    Nametag() {}
    
    // Divvy can take advantage of overloaded constructors, so let's add one!
    Nametag(const std::string& name) : m_name(name) {}
    
    // The update method provides functionality to Components.
    virtual void update()
    {
        std::cout << "Hello! My name is " << m_name << ".\n";
    }
    
    // The clone method allows copy functionality when copying Components between Entities.
    virtual void clone(const divvy::Component& other)
    {
        // Copy over name
        m_name = static_cast<const Nametag&>(other).m_name;
    }
    
    // Setter method with self return
    Nametag& setName(const std::string& name)
    {
        m_name = name;
        return *this;
    }
    
    // Getter method
    const std::string& getName() { return m_name; }
    
private:
    std::string m_name;
};
```

#### Adding Components to a World

Once we have a valid component, we have to add the component type to a `World`.

The `World` contains all of the possible component types that you can add to an `Entity`. Thus, it is important that we add all the component types that we want into the `World` before we assign any components to an `Entity`.

```C++
divvy::World world;    // Create a World (collection of Components and Entity associations)
world.add<Nametag>();  // Add the component type to the World
```

Note that different Worlds can have different component types, this is mainly for customizing which Components you want to include in a World. **When copying entities between two different worlds, only the component types that exist in both worlds will be copied over.** More on this in the [*Copying/Moving Entities*](#copying-moving-entities) section.

#### Creating an Entity

`Entity` is the interface to add, remove, and retrieve components. To act as this interface, Entities have to be assigned to a `World`, since the `World` is what holds all of the Components. Keep in mind that if there is no `World` assigned, the `Entity` is considered to be invalid and won't be of any use. Trying to use an invalid `Entity` will result in an exception being thrown.

Now that we have a component type in a `World`, we can start using it by creating an `Entity` in the `World`. There are two ways that can accomplish this:

Either by passing a `World` to the constructor,
```C++
divvy::Entity hero(world);
```
or by passing a `World` to the `reset(...)` method.
```C++
divvy::Entity hero;
hero.reset(world);
```

Although these two ways are equivalent, the reset method is most useful when creating an array of `Entity`. In which case Entities would be created using the default constructor which doesn't assign a `World`. **Note that there is a corresponding `reset` method for every constructor of `Entity`.**

#### Adding Components to an Entity

When Components are added to Entities, the constructor that matches the parameter list will be called. This allows for overloaded constructors to be utilized.

```C++
hero.add<Nametag>("Mario"); // Calls the overloaded constructor of Nametag
```

#### Updating the World

Whenever a `World` is updated, all of the Components that are active inside the `World` are updated as well.

```C++
world.update(); // OUTPUT: Hello! My name is Mario.
```

#### Retrieving Components from an Entity

The `Entity` interface also makes retrieving components as easy as adding them.

```C++
player.get<Nametag>().setName("Luigi");

world.update(); // OUTPUT: Hello! My name is Luigi.
```

#### Copying/Moving Entities

Entities have the additional functionality to be copable and movable. However, it is important to note that copying and moving are fundamentially different.
  - **Copying**: calls the `clone` method, which copies the specified variables
  - **Moving**: moves the reference of the `Entity`, leaving the other `Entity` invalid

```C++
// Copying
divvy::Entity enemy(player);
enemy.get<Nametag>().setName("Bowser");

// Moving
divvy::Entity player2 = std::move(player);
player2.get<Nametag>().setName("Luigi");

// Check if the move was successful
if (player.valid() == false)
{
    world.update();
}

/* OUTPUT:
Hello! My name is Luigi.
Hello! My name is Bowser.
*/
```

#### Checking for Components

```C++
if (enemy.has<Nametag>() == true)
{
    std::cout << "Enemy has a name! \n";
}
```

It can serve useful to check whether an `Entity` has a specific `Component`. The `has` method allows for this check to be possible. This is especially useful to check whether an `Entity` has the required Components before adding another specialized `Component`.

```C++
class Physics : public divvy::Component
{
    Physics()
    {
        if (m_entity->has<Transform>() == false || 
            m_entity->has<Mass>() == false)
        {
            std::cout << "Physics requires Transform and Mass to already be present. \n";
            m_entity->remove<Physics>();
        }
    }
    
    ...
}
```

**`m_entity` is a protected pointer built into `Component` that points to the assigned `Entity`.**

#### Removing Components

```C++
enemy.remove<Nametag>();

world.update(); // OUTPUT: Hello! My name is Luigi.
```

This immediately deactives the Component and removes it from the `Entity` that it is assigned to.

#### Putting It All Together

```C++
#include <iostream>
#include <string>

#include "divvy.hpp" // or where ever it is located

class Nametag : public divvy::Component
{
public:
    // Default constructor
    Nametag() {}
    
    // Divvy can take advantage of overloaded constructors, so let's add one!
    Nametag(const std::string& name) : m_name(name) {}
    
    // The update method provides functionality to Components.
    virtual void update()
    {
        std::cout << "Hello! My name is " << m_name << ".\n";
    }
    
    // The clone method allows copy functionality when copying Components between Entities.
    virtual void clone(const divvy::Component& other)
    {
        // Copy over name
        m_name = static_cast<const Nametag&>(other).m_name;
    }
    
    // Setter method with self return
    Nametag& setName(const std::string& name)
    {
        m_name = name;
        return *this;
    }
    
    // Getter method
    const std::string& getName() { return m_name; }
    
private:
    std::string m_name;
};

int main()
{
    divvy::Entity hero(world);
    hero.add<Nametag>("Mario"); // Calls the overloaded constructor of Nametag
    
    world.update();
    
    player.get<Nametag>().setName("Luigi");

    world.update();
    
    // Copying
    divvy::Entity enemy(player);
    enemy.get<Nametag>().setName("Bowser");
    
    // Moving
    divvy::Entity player2 = std::move(player);
    player2.get<Nametag>().setName("Luigi");
    
    // Check if the move was successful
    if (player.valid() == false)
    {
        world.update();
    }
    
    if (enemy.has<Nametag>() == true)
    {
        std::cout << "Enemy has a name! \n";
    }
    
    enemy.remove<Nametag>();

    world.update();
}
```
