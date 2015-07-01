#include <iostream>

#include "divvy.hpp"
using namespace divvy;

// Example of a Component
class Transform : public Component
{
public:
    virtual void update()
    {
        //std::cout << "Transform: (" << x << "," << y << ")" << std::endl;
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
        //std::cout << "Name: " << name << std::endl;
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

    {
        // Create 500,000 Entities
        Entity e[500000];
        for (int i = 0; i < 500000; i++)
        {
            e[i].reset(w);
            e[i].add<Transform>().setX(6).setY(9);
            e[i].add<Nametag>().setName("Sam");
        }
        std::cout << "Created" << std::endl;
        w.update();
        std::cout << "Updated" << std::endl;
    }

    // Entities are now out of scope
    std::cout << "Done" << std::endl;
}
