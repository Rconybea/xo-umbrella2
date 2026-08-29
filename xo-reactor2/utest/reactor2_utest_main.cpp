/** @file reactor2_utest_main.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

// note: do NOT define CATCH_CONFIG_RUNNER/CATCH_CONFIG_MAIN here.  The catch2
//       implementation (registry + session runner) is compiled once, in
//       libxo_testutil (UtestAppStart.cpp).  See ratio_utest_main.cpp.
#define CATCH_CONFIG_EXTERNAL_INTERFACES // before UtestListener.hpp

#include <xo/ppsink/PpStyle.hpp>
#include <xo/testutil/UtestAppStart.hpp>
#include <xo/testutil/UtestListener.hpp>

namespace xo {
    CATCH_REGISTER_LISTENER(UtestListener);
}

int
main(int argc, char* argv[])
{
    /* unit tests pin rendered TEXT, so must not be handed color escapes */
    xo::pp::PpStyle::default_style() = xo::pp::PpStyle::plain();

    auto app = xo::UtestAppStart("utest.reactor2");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

    app.setup();

    return app.run();
}

/* end reactor2_utest_main.cpp */
