#include <iostream>
#include <string>

#include "divvy.hpp"

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
        m_name = divvy::cast<Nametag>(other).m_name;
    }

    // Setter method with self return
    Nametag& setName(const std::string& name)
    {
        m_name = name;
        return *this;
    }

private:
    std::string m_name;
};

int main()
{
    divvy::World world;         // Create a World
    world.add<Nametag>();       // Add the component type to the World

    divvy::Entity hero(world);
    hero.add<Nametag>("Mario"); // Calls the overloaded constructor

    world.update();             // OUTPUT: Hello! My name is Mario.

    hero.get<Nametag>().setName("Luigi");

    world.update();             // OUTPUT: Hello! My name is Luigi.

    // Copying
    divvy::Entity enemy(hero);
    enemy.get<Nametag>().setName("Bowser");

    // Moving
    divvy::Entity princess = std::move(hero);
    princess.get<Nametag>().setName("Peach");

    // Check if the move was successful
    if (hero.valid() == false)
    {
        world.update();

        /* OUTPUT
        Hello! My name is Peach.
        Hello! My name is Bowser.
        */
    }

    if (enemy.has<Nametag>() == true)
    {
        std::cout << "Enemy has a name! \n";
    }

    enemy.remove<Nametag>();

    if (enemy.has<Nametag>() == false)
    {
        std::cout << "Enemy no longer has a name! \n";
    }

    world.update();             // OUTPUT: Hello! My name is Peach.
}

/* COMBINED OUTPUT
Hello! My name is Mario.
Hello! My name is Luigi.
Hello! My name is Peach.
Hello! My name is Bowser.
Enemy has a name!
Enemy no longer has a name!
Hello! My name is Peach.
*/

