/* file alloc2_utest_main.cpp */

#define CATCH_CONFIG_RUNNER // before UtestListener.hpp

#include <xo/testutil/UtestAppStart.hpp>
#include <xo/testutil/UtestListener.hpp>

namespace xo {
    CATCH_REGISTER_LISTENER(UtestListener);
}

int
main(int argc, char* argv[])
{
    return xo::UtestAppStart("utest.alloc2").run(argc, argv);
}

/* end alloc2_utest_main.cpp */
