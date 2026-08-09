/** @file reflect_utest_main.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

// note: do NOT define CATCH_CONFIG_RUNNER/CATCH_CONFIG_MAIN here.  The catch2
//       implementation (registry + session runner) is compiled once, in
//       libxo_testutil (UtestAppStart.cpp).  Defining CATCH_CONFIG_RUNNER here
//       would compile a second copy of the catch2 runtime into this executable,
//       giving it its own test registry.  On linux ELF symbol interposition
//       merges the two; on osx the two-level namespace keeps them separate, so
//       the runner (in the dylib) sees an empty registry -> "No tests ran".
//
//       CATCH_CONFIG_EXTERNAL_INTERFACES pulls in just the reporter/listener
//       interfaces (Catch::TestEventListenerBase + CATCH_REGISTER_LISTENER)
//       needed by UtestListener.hpp, WITHOUT the runtime implementation.
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
    /* Unit tests pin rendered TEXT, so they must not be handed color escapes.
     * PpStyle's defaults are the legacy ones -- grey tag names, yellow struct
     * field names (xo/ppsink/PpStyle.hpp) -- right for a terminal, useless in
     * an expectation string.  A test that wants color asks locally, via
     * default_style_guard or sink.with_style().
     */
    xo::pp::PpStyle::default_style() = xo::pp::PpStyle::plain();

    auto app = xo::UtestAppStart("utest.reflect");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

    app.setup();

    return app.run();
}

/* end reflect_utest_main.cpp */
