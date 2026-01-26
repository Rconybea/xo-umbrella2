/* file procedure2_utest_main.cpp */

#include <xo/subsys/Subsystem.hpp>

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

int
main(int argc, char* argv[])
{
    using xo::Subsystem;

    Subsystem::initialize_all();

    int result = Catch::Session().run(argc, argv);

    return result;
}

/* end procedure2_utest_main.cpp */
