/** @file websock_unit_main.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  main() for xo-websock's unit tests.  Named websock_UNIT_main rather than
 *  the usual websock_utest_main because that name belongs to the kalman-era
 *  browser demo sharing this directory -- see utest/CMakeLists.txt and
 *  .xo-backlog/xo-websock/issues/01.
 **/

#define CATCH_CONFIG_EXTERNAL_INTERFACES // before UtestListener.hpp

#include "WebsockUtestAppcx.hpp"
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
    using xo::WebsockUtestAppcx;
    using xo::Indentlog2Config;
    using xo::pp::PpConfig;

    /* unit tests pin rendered TEXT, so no color escapes */
    xo::pp::PpStyle::default_style() = xo::pp::PpStyle::plain();

    auto app = xo::UtestAppStart("utest.websock");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

    using UtestAppConfig = WebsockUtestAppcx::UtestAppConfig;

    UtestAppConfig utest_cfg{ Indentlog2Config(PpConfig::plain(),
                                               c_temp_arena_capacity),
                              xo::ReflectConfig(),
                              xo::PrintJsonConfig(),
                              xo::WebsockConfig() };

    WebsockUtestAppcx::configure(utest_cfg);

    app.setup(); // calls Subsystem::initialize_all()

    return app.run();
}

/* end websock_unit_main.cpp */
