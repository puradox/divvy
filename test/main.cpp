#include <iostream>

#include "gtest/gtest.h"

//#define DIVVY_DEBUG
#include "divvy.hpp"

using namespace divvy;

//=============================[ Fixture Testing Class ]=================================

class DivvyTest : public ::testing::Test
{
protected:
    World world;
};

//=============================[ Component Example #1 ]==================================

class Transform : public Component
{
public:
    Transform() {}

    Transform(int x, int y) : m_x(x), m_y(y) {}

    virtual void update()
    {
        #ifdef DIVVY_DEBUG
        std::cout << "Old Transform: (" << m_x << "," << m_y << ")" << std::endl;
        #endif

        m_x++;
        m_y++;

        #ifdef DIVVY_DEBUG
        std::cout << "New Transform: (" << m_x << "," << m_y << ")" << std::endl;
        #endif
    }

    virtual void clone(const Component& other)
    {
        auto& derived = static_cast<const Transform&>(other);

        m_x = derived.m_x;
        m_y = derived.m_y;
    }

    Transform& setX(int x)
    {
        m_x = x;
        return *this;
    }

    Transform& setY(int y)
    {
        m_y = y;
        return *this;
    }

    int getX() { return m_x; }
    int getY() { return m_y; }

private:
    int m_x = 0, m_y = 0;
};

//=============================[ Component Example #2 ]==================================

class Nametag : public Component
{
public:
    Nametag() {}

    Nametag(const std::string& name) : m_name(name) {}

    virtual void update()
    {
        #ifdef DIVVY_DEBUG
        std::cout << "Name: " << m_name << std::endl;
        #endif
    }

    virtual void clone(const Component& other)
    {
        m_name = static_cast<const Nametag&>(other).m_name;
    }

    Nametag& setName(const std::string& name)
    {
        m_name = name;
        return *this;
    }

    const std::string& getName() { return m_name; }

private:
    std::string m_name;
};

//================================[ World Register ]=====================================

TEST_F(DivvyTest, WorldHasComponentRegistered)
{
    ASSERT_FALSE(world.has<Transform>());
}

TEST_F(DivvyTest, WorldRegisterComponent)
{
    world.add<Transform>();
    ASSERT_TRUE(world.has<Transform>());
}

TEST_F(DivvyTest, WorldUnregisterComponent)
{
    world.add<Transform>();
    ASSERT_TRUE(world.has<Transform>());

    world.remove<Transform>();
    ASSERT_FALSE(world.has<Transform>());
}

//==================================[ World Clear ]======================================

TEST_F(DivvyTest, ClearComponents)
{
    world.add<Transform>();
    ASSERT_TRUE(world.has<Transform>());

    world.clear();
    ASSERT_FALSE(world.has<Transform>());
}

TEST_F(DivvyTest, ClearComponentsAndEntites)
{
    world.add<Transform>();
    ASSERT_TRUE(world.has<Transform>());

    world.clear();
    ASSERT_FALSE(world.has<Transform>());
}

//============================[ Entity Basic Constructors ]==============================

TEST_F(DivvyTest, CreateNullEntity)
{
    Entity ball;
    ASSERT_FALSE(ball.valid());
}

TEST_F(DivvyTest, CreateEntityWorld)
{
    Entity ball(world);
    ASSERT_TRUE(ball.valid());
}

//================================[ Entity Null Calls ]==================================

TEST_F(DivvyTest, EntityNullAdd)
{
    Entity player;
    ASSERT_THROW(player.add<Transform>(), std::runtime_error);
}

TEST_F(DivvyTest, EntityNullGet)
{
    Entity player;
    ASSERT_THROW(player.get<Transform>(), std::runtime_error);
}

TEST_F(DivvyTest, EntityNullRemove)
{
    Entity player;
    ASSERT_THROW(player.remove<Transform>(), std::runtime_error);
}

//================================[ Entity Component ]===================================

TEST_F(DivvyTest, EntityAddComponent)
{
    world.add<Transform>();

    Entity player(world);
    player.add<Transform>();

    ASSERT_TRUE(player.has<Transform>());
}

TEST_F(DivvyTest, EntityAddNullComponent)
{
    Entity player(world);

    ASSERT_THROW(player.add<Transform>(), std::runtime_error);
}

TEST_F(DivvyTest, EntityGetComponent)
{
    world.add<Transform>();

    Entity player(world);
    player.add<Transform>(1, 2);

    ASSERT_TRUE(player.has<Transform>());
    ASSERT_EQ(1, player.get<Transform>().getX());
    ASSERT_EQ(2, player.get<Transform>().getY());
}

TEST_F(DivvyTest, EntityGetNullComponent)
{
    world.add<Transform>();

    Entity player(world);

    ASSERT_FALSE(player.has<Transform>());
    ASSERT_THROW(player.get<Transform>(), std::runtime_error);
}

TEST_F(DivvyTest, EntityRemoveComponent)
{
    world.add<Transform>();

    Entity player(world);

    player.add<Transform>();
    ASSERT_TRUE(player.has<Transform>());

    player.remove<Transform>();
    ASSERT_FALSE(player.has<Transform>());
}

TEST_F(DivvyTest, EntityRemoveNullComponent)
{
    world.add<Transform>();

    Entity player(world);

    ASSERT_FALSE(player.has<Transform>());
    player.remove<Transform>();
    ASSERT_FALSE(player.has<Transform>());
}

//===================================[ Entity Copy ]=====================================

TEST_F(DivvyTest, EntityCopy)
{
    world.add<Transform>();

    Entity player(world);
    player.add<Transform>(1,2);

    Entity copy(player);

    ASSERT_TRUE(copy.valid());
    ASSERT_EQ(1, copy.get<Transform>().getX());
    ASSERT_EQ(2, copy.get<Transform>().getY());
}

TEST_F(DivvyTest, EntityCopyOtherWorld)
{
    // World 1
    world.add<Transform>();
    world.add<Nametag>();

    // Player
    Entity player(world);
    player.add<Transform>(1,2);
    player.add<Nametag>("player");

    // World 2
    World world2;
    world2.add<Transform>();

    // Copy
    Entity copy(player, world2);

    // Test
    ASSERT_TRUE(copy.valid());
    ASSERT_TRUE(copy.has<Transform>());
    ASSERT_FALSE(copy.has<Nametag>());

    ASSERT_EQ(1, copy.get<Transform>().getX());
    ASSERT_EQ(2, copy.get<Transform>().getY());
}

//==================================[ Entity Reset ]=====================================

TEST_F(DivvyTest, EntityReset)
{
    Entity player(world);
    player.reset();

    ASSERT_FALSE(player.valid());
}

TEST_F(DivvyTest, EntityResetWorld)
{
    Entity player;
    player.reset(world);
    ASSERT_TRUE(player.valid());
}

TEST_F(DivvyTest, EntityResetCopy)
{
    world.add<Transform>();

    Entity player(world);
    player.add<Transform>(1,2);

    Entity enemy;
    enemy.reset(player);

    ASSERT_EQ(1, enemy.get<Transform>().getX());
    ASSERT_EQ(2, enemy.get<Transform>().getY());
}

TEST_F(DivvyTest, EntityResetCopyOtherWorld)
{
    world.add<Transform>();
    world.add<Nametag>();

    Entity player(world);
    player.add<Transform>(1,2);
    player.add<Nametag>("player");

    ASSERT_TRUE(player.has<Transform>());
    ASSERT_TRUE(player.has<Nametag>());

    World world2;
    world2.add<Transform>();

    Entity copy;
    copy.reset(player, world2);

    ASSERT_TRUE(copy.has<Transform>());
    ASSERT_FALSE(copy.has<Nametag>());

    ASSERT_EQ(1, copy.get<Transform>().getX());
    ASSERT_EQ(2, copy.get<Transform>().getY());
}

//==================================[ World Update ]=====================================

TEST_F(DivvyTest, WorldUpdate)
{
    world.add<Transform>();

    Entity player(world);
    player.add<Transform>(1, 2);

    ASSERT_TRUE(player.has<Transform>());
    ASSERT_EQ(1, player.get<Transform>().getX());
    ASSERT_EQ(2, player.get<Transform>().getY());

    world.update();

    ASSERT_EQ(2, player.get<Transform>().getX());
    ASSERT_EQ(3, player.get<Transform>().getY());
}

//=================================[ Multiple Worlds ]===================================

TEST_F(DivvyTest, MultipleWorlds)
{

}

//=======================================[ Main ]========================================

int main(int argc, char** argv)
{
    World w;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
