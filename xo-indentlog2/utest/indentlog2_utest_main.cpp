/** @file indentlog2_utest_main.cpp
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

#include <xo/indentlog2/appcx_indentlog2.hpp>
#include <xo/indentlog2/config_indentlog2.hpp>
#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/ppsink/PpStyle.hpp>
//#include <xo/subsys/AppContext.hpp>
#include <xo/testutil/UtestAppStart.hpp>
#include <xo/testutil/UtestListener.hpp>

namespace xo {
    CATCH_REGISTER_LISTENER(UtestListener);
}

namespace {
    /** capacity for the thread-local scratch arena behind tostr()/toppstr().
     **/
    constexpr std::uint32_t c_temp_arena_capacity = 64 * 1024;

}

int
main(int argc, char* argv[])
{
    using UtestAppConfig  = xo::AppConfig<xo::S_indentlog2_tag>;
    using UtestAppContext = xo::AppContext<xo::S_indentlog2_tag>;
    using xo::Indentlog2_Config;
    using xo::pp::PpConfig;

    /* Unit tests pin rendered TEXT, so they must not be handed color escapes.
     * PpStyle's defaults are the legacy ones -- grey tag names, yellow struct
     * field names (xo/ppsink/PpStyle.hpp) -- right for a terminal, useless in
     * an expectation string.  A test that wants color asks locally, via
     * default_style_guard or sink.with_style().
     */
    xo::pp::PpStyle::default_style() = xo::pp::PpStyle::plain();

    auto app = xo::UtestAppStart("utest.indentlog2");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

    UtestAppConfig utest_config{ Indentlog2_Config(PpConfig::plain(),
                                                   c_temp_arena_capacity) };
    UtestAppContext utest_appcx{ utest_config };

    app.setup();

    return app.run();
}

/* end indentlog2_utest_main.cpp */
