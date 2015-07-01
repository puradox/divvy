#include <iostream>

#include "divvy.hpp"
using namespace divvy;

// Example of a Component
class Transform : public Component
{
public:
    virtual void update()
    {
        std::cout << "Transform: (" << x << "," << y << ")" << std::endl;
    }

    virtual std::shared_ptr<Component> clone() const
    {
        return std::make_shared<Transform>(*this);
    }

    Transform& setX(int x)
    {
        this->x = x;
        return *this;
    }

    Transform& setY(int y)
    {
        this->y = y;
        return *this;
    }

private:
    int x = 0, y = 0;
};

class Nametag : public Component
{
public:
    virtual void update()
    {
        std::cout << "Name: " << name << std::endl;
    }

    virtual std::shared_ptr<Component> clone() const
    {
        return std::make_shared<Nametag>(*this);
    }

    Nametag& setName(const std::string& name)
    {
        this->name = name;
        return *this;
    }

    const std::string& getName() const
    {
        return name;
    }

private:
    std::string name;
};

int main()
{
    // Make the world
    World w;

    // Register Components
    w.registerComponent<Transform>();
    w.registerComponent<Nametag>();

    // Make a Player
    Entity player(w);
    player.add<Transform>().setX(8).setY(8);
    player.add<Nametag>().setName("Sam");

        std::cout << "\n\n";
        w.update();
        std::cout << "\n\n";

    // Make an Enemy
    Entity enemy(player);
    enemy.get<Nametag>().setName("Bowser");

        std::cout << "\n\n";
        w.update();
        std::cout << "\n\n";
}
