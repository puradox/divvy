#include <iostream>

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

//=======================================[ Main ]========================================

int main(int argc, char** argv)
{
	int result = Catch::Session().run(argc, argv);

	std::cin.ignore();
	return result;
}