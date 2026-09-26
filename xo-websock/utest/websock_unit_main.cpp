/** @file websock_unit_main.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  main() for xo-websock's unit tests.  Named websock_UNIT_main rather than
 *  the usual websock_utest_main because that name belongs to the kalman-era
 *  browser demo sharing this directory -- see utest/CMakeLists.txt and
 *  .xo-backlog/xo-websock/issues/01.
 **/

// note: do NOT define CATCH_CONFIG_RUNNER/CATCH_CONFIG_MAIN here.  The catch2
//       implementation is compiled once, in libxo_testutil (UtestAppStart.cpp);
//       a second copy here would get its own test registry, which on osx the
//       runner never sees.  CATCH_CONFIG_EXTERNAL_INTERFACES pulls in just the
//       listener interfaces UtestListener.hpp needs.
#define CATCH_CONFIG_EXTERNAL_INTERFACES // before UtestListener.hpp

#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <xo/indentlog2/cx/Indentlog2Config.hpp>
#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/testutil/UtestAppStart.hpp>
#include <xo/testutil/UtestListener.hpp>
#include <xo/ppsink/PpStyle.hpp>

namespace xo {
    CATCH_REGISTER_LISTENER(UtestListener);
}

namespace {
    /** capacity for the thread-local scratch arena behind tostr()/toppstr() **/
    constexpr std::uint32_t c_temp_arena_capacity = 64 * 1024;
}

int
main(int argc, char* argv[])
{
    using UtestAppConfig  = xo::AppConfig<xo::S_indentlog2_tag>;
    using UtestAppContext = xo::AppContext<xo::S_indentlog2_tag>;
    using xo::Indentlog2Config;
    using xo::pp::PpConfig;

    /* unit tests pin rendered TEXT, so no color escapes */
    xo::pp::PpStyle::default_style() = xo::pp::PpStyle::plain();

    auto app = xo::UtestAppStart("utest.websock");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

    UtestAppConfig utest_config{ Indentlog2Config(PpConfig::plain(),
                                                  c_temp_arena_capacity) };
    UtestAppContext utest_appcx{ utest_config };

    app.setup(); // calls Subsystem::initialize_all()

    return app.run();
}

/* end websock_unit_main.cpp */
