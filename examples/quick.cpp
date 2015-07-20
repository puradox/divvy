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
