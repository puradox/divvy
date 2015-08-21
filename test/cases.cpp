#ifndef DIVVYTEST_HPP
#define DIVVYTEST_HPP

#include "catch.hpp"

//#define DIVVY_DEBUG
#include "divvy.hpp"
using namespace divvy;


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
		auto& derived = cast<Transform>(other);

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


//================================[ Test Cases ]=========================================


TEST_CASE("World can manipulate Components", "[world][component]")
{
	World world;

	SECTION("detecting nonexistent Component")
	{
		REQUIRE_FALSE(world.has<Transform>());
	}

	SECTION("adding a Component")
	{
		world.add<Transform>();
		REQUIRE(world.has<Transform>());

		world.add<Transform>();
		REQUIRE(world.has<Transform>());
	}

	SECTION("removing a Component")
	{
		world.remove<Transform>();
		REQUIRE_FALSE(world.has<Transform>());
	}
}


TEST_CASE("World can clear", "[world][component]")
{
	World world;

	SECTION("clearing Components")
	{
		world.add<Transform>();
		REQUIRE(world.has<Transform>());

		world.clear();
		REQUIRE_FALSE(world.has<Transform>());
	}

	SECTION("clearing Entities")
	{
		// TODO
	}
}


TEST_CASE("Entities can initialize", "[entity]")
{
	SECTION("creating a null Entity")
	{
		Entity ball;
		REQUIRE_FALSE(ball.valid());
	}

	SECTION("creating a Entity in a World")
	{
		World world;
		Entity ball(world);
		REQUIRE(ball.valid());
	}
}


TEST_CASE("Entity can add Components", "[entity][component]")
{
	World world;
	Entity entity;

	SECTION("adding a Component without a World")
	{
		REQUIRE_THROWS_AS(entity.add<Transform>(), std::runtime_error);
	}

	entity.reset(world);

	SECTION("adding a Component that isn't added to a World")
	{
		REQUIRE_THROWS_AS(entity.add<Transform>(), std::runtime_error);
	}

	world.add<Transform>();
	entity.add<Transform>();

	SECTION("adding a Component")
	{
		REQUIRE(entity.has<Transform>());
	}
}


TEST_CASE("Entity can get Components", "[entity][component]")
{
	World world;
	Entity entity;

	
	SECTION("getting a Component without a World")
	{
		REQUIRE_THROWS_AS(entity.get<Transform>(), std::runtime_error);
	}

	entity.reset(world);
	
	SECTION("getting a Component that isn't added to a World")
	{
		REQUIRE_THROWS_AS(entity.get<Transform>(), std::runtime_error);
	}

	world.add<Transform>();
	
	SECTION("getting a Component not added to the Entity")
	{
		REQUIRE_FALSE(entity.has<Transform>());
		REQUIRE_THROWS_AS(entity.get<Transform>(), std::runtime_error);
	}

	SECTION("getting a Component")
	{
		entity.add<Transform>(1, 2);

		REQUIRE(entity.has<Transform>());
		REQUIRE(entity.get<Transform>().getX() == 1);
		REQUIRE(entity.get<Transform>().getY() == 2);
	}
}


TEST_CASE("Entity can remove Components", "[entity][component]")
{
	World world;
	Entity entity;

	SECTION("removing a Component without a World")
	{
		REQUIRE_THROWS_AS(entity.remove<Transform>(), std::runtime_error);
	}

	entity.reset(world); 
	
	SECTION("removing a Component that isn't added to a World")
	{
		REQUIRE_THROWS_AS(entity.remove<Transform>(), std::runtime_error);
	}

	world.add<Transform>();

	SECTION("removing a Component that isn't added to the Entity")
	{
		REQUIRE_FALSE(entity.has<Transform>());
		entity.remove<Transform>();
		REQUIRE_FALSE(entity.has<Transform>());
	}

	SECTION("removing a Component")
	{
		entity.add<Transform>();
		REQUIRE(entity.has<Transform>());

		entity.remove<Transform>();
		REQUIRE_FALSE(entity.has<Transform>());
	}
}


TEST_CASE("Entity is copyable", "[entity]")
{
	World world;
	Entity entity(world);

	world.add<Transform>();
	entity.add<Transform>(1, 2);

	SECTION("copying using the copy constructor")
	{
		Entity copy(entity);

		REQUIRE(copy.valid());
		REQUIRE(copy.get<Transform>().getX() == 1);
		REQUIRE(copy.get<Transform>().getY() == 2);
	}
	
	SECTION("copying using the equals operator")
	{
		Entity copy = entity;

		REQUIRE(copy.valid());
		REQUIRE(copy.get<Transform>().getX() == 1);
		REQUIRE(copy.get<Transform>().getY() == 2);
	}
}


TEST_CASE("Entity is copyable across Worlds", "[entity][world]")
{
	World w1, w2;
	Entity entity(w1);

	w1.add<Transform>();
	w1.add<Nametag>();

	entity.add<Transform>(1, 2);
	entity.add<Nametag>("Divvy");

	w2.add<Transform>();

	Entity copy(entity, w2);

	REQUIRE(copy.valid());
	REQUIRE(copy.has<Transform>());
	REQUIRE_FALSE(copy.has<Nametag>());

	REQUIRE(copy.get<Transform>().getX() == 1);
	REQUIRE(copy.get<Transform>().getY() == 2);
}


TEST_CASE("Entity is movable", "[entity]")
{
	World world;
	Entity entity(world);

	world.add<Transform>();
	entity.add<Transform>(1, 2);

	SECTION("copying using the copy constructor")
	{
		Entity copy(std::move(entity));

		REQUIRE_FALSE(entity.valid());

		REQUIRE(copy.valid());
		REQUIRE(copy.get<Transform>().getX() == 1);
		REQUIRE(copy.get<Transform>().getY() == 2);
	}

	SECTION("copying using the equals operator")
	{
		Entity copy = std::move(entity);

		REQUIRE_FALSE(entity.valid());

		REQUIRE(copy.valid());
		REQUIRE(copy.get<Transform>().getX() == 1);
		REQUIRE(copy.get<Transform>().getY() == 2);
	}
}


TEST_CASE("Entity can reset", "[entity][world]")
{
	World world;
	Entity entity;

	entity.reset();

	SECTION("resetting from null")
	{
		REQUIRE(entity.valid() == false);
	}

	entity.reset(world);

	SECTION("resetting World")
	{
		REQUIRE(entity.valid() == true);
	}
	
	world.add<Transform>();
	world.add<Nametag>();

	entity.add<Transform>(1, 2);
	entity.add<Nametag>("Divvy");

	SECTION("resetting by copy")
	{
		Entity copy;
		copy.reset(entity);

		REQUIRE(copy.valid());
		REQUIRE(copy.has<Transform>());

		REQUIRE(copy.get<Transform>().getX() == 1);
		REQUIRE(copy.get<Transform>().getY() == 2);
	}

	SECTION("resetting by copying between Worlds")
	{
		World alt_world;
		Entity copy;

		alt_world.add<Nametag>();

		copy.reset(entity, alt_world);

		REQUIRE(copy.valid());
		REQUIRE(copy.has<Nametag>());
		REQUIRE_FALSE(copy.has<Transform>());

		REQUIRE(copy.get<Nametag>().getName().compare("Divvy") == 0);
	}
}


TEST_CASE("World can update Entities", "[world][entity]")
{
	World world;
	Entity entity(world);

	world.add<Transform>();
	entity.add<Transform>(1, 2);

	REQUIRE(entity.has<Transform>());
	REQUIRE(entity.get<Transform>().getX() == 1);
	REQUIRE(entity.get<Transform>().getY() == 2);

	world.update();

	REQUIRE(entity.get<Transform>().getX() == 2);
	REQUIRE(entity.get<Transform>().getY() == 3);
}


#endif // DIVVYTEST_HPP