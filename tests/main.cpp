#include <iostream>

#define DIVVY_DEBUG
#include "divvy.hpp"
using namespace divvy;

// Example of a Component
class Transform : public Component
{
public:
    virtual void update()
    {
        std::cout << x << "   " << y << std::endl;
    }

    virtual std::shared_ptr<Component> clone() const
    {
        std::shared_ptr<Transform> clone = std::make_shared<Transform>();
        clone->setX(x).setY(y);

        return std::dynamic_pointer_cast<Component>(clone);
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
        std::cout << "Hello! My name is " << name << std::endl;
    }

    virtual std::shared_ptr<Component> clone() const
    {
        std::shared_ptr<Nametag> clone = std::make_shared<Nametag>();
        clone->name = name;

        return std::dynamic_pointer_cast<Component>(clone);
    }

    Nametag& setName(const std::string& name)
    {
        this->name = name;
        return *this;
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
    Entity player = w.addEntity();
    w.addComponent<Transform>(player).setX(5).setY(10);
    w.addComponent<Nametag>(player).setName("Sam");

    // Make a Enemy based on the clone of Player
    Entity enemy = w.addEntity(player);
    w.getComponent<Nametag>(enemy).setName("Browser");

    // Update
    std::cout << "update #1" << std::endl;
    w.update();
    std::cout << std::endl;

    // Remove Player
    w.removeEntity(player);

    // Update
    std::cout << "update #2" << std::endl;
    w.update();
    std::cout << std::endl;

    // Remove Enemy
    w.removeEntity(enemy);
}
