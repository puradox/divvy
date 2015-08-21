#include <iostream>

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

//=======================================[ Main ]========================================

int main(int argc, char** argv)
{
	return Catch::Session().run(argc, argv);
}