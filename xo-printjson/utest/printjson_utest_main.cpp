/** @file printjson_utest_main.cpp
 *
 *  @author Roland Conybeare, Sep 2026
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

#include "printjson_utest_appcx.hpp"
#include <xo/facet/cx/FacetConfig.hpp>
#include <xo/facet/init_facet.hpp>
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <xo/indentlog2/cx/Indentlog2Config.hpp>
#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/testutil/UtestAppStart.hpp>
#include <xo/testutil/UtestListener.hpp>

namespace xo {
    CATCH_REGISTER_LISTENER(UtestListener);
}

namespace {
    /** capacity for the thread-local scratch arena behind tostr()/toppstr().
     **/
    constexpr std::uint32_t c_temp_arena_capacity = 64 * 1024;

    using UtestAppConfig  = xo::AppConfig<xo::S_indentlog2_tag, xo::S_facet_tag>;
    using UtestAppContext = xo::AppContext<xo::S_indentlog2_tag, xo::S_facet_tag>;

    /** the one context chain for this executable.
     *
     *  A function-local static rather than a local in main() so that
     *  xo::printjson_utest_facet_appcx() can hand tests the facet half -- see
     *  printjson_utest_appcx.hpp for why they cannot just build their own.
     **/
    UtestAppContext & utest_appcx() {
        /* Unit tests pin rendered TEXT, so they must not be handed color
         * escapes.  PpStyle's defaults are the legacy ones, right for a
         * terminal and useless in an expectation string.  A test that wants
         * color asks locally, via default_style_guard or sink.with_style().
         */
        static UtestAppContext s_cx{
            UtestAppConfig(xo::Indentlog2Config(xo::pp::PpConfig::plain(),
                                                c_temp_arena_capacity),
                           xo::FacetConfig::make_default()) };

        return s_cx;
    }
}

namespace xo {
    FacetAppcx &
    printjson_utest_facet_appcx()
    {
        return utest_appcx().cx<S_facet_tag>();
    }
} /*namespace xo*/

int
main(int argc, char* argv[])
{
    /* Unit tests pin rendered TEXT, so they must not be handed color escapes.
     * PpStyle's defaults are the legacy ones, right for a terminal and useless
     * in an expectation string.  Matters here once printjson writes into a
     * PpSink rather than a std::ostream (.xo-backlog/xo-printjson/issues/01).
     * A test that wants color asks locally, via default_style_guard or
     * sink.with_style().
     */
    xo::pp::PpStyle::default_style() = xo::pp::PpStyle::plain();

    auto app = xo::UtestAppStart("utest.printjson");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

    /* builds the whole chain, indentlog2 then facet.  A test reaches the facet
     * half through xo::printjson_utest_facet_appcx().
     */
    (void)utest_appcx();

    app.setup(); // calls Subsystem::initialize_all()

    return app.run();
}

/* end printjson_utest_main.cpp */
