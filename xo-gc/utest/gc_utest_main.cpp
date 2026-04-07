/* file gc_utest_main.cpp */

#include <xo/gc/init_gc.hpp>
#include <xo/subsys/Subsystem.hpp>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

using xo::S_gc_tag;
using xo::InitSubsys;
using xo::InitEvidence;

// ensure xo-gc properly initialized when Subsystem::initialize_all() runs
static InitEvidence s_init = (InitSubsys<S_gc_tag>::require());

int
main(int argc, char* argv[])
{
    using xo::Subsystem;

    // Your custom initialization code here
    Subsystem::initialize_all();

    // Run Catch2's test session
    int result = Catch::Session().run(argc, argv);

    // cleanup here, if any

    return result;
}

/* end gc_utest_main.cpp */
