/* file object2_utest_main.cpp */

#include "object2_utest_appcx.hpp"
#include <xo/facet/cx/FacetConfig.hpp>
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <xo/indentlog2/cx/Indentlog2Config.hpp>
#include <xo/subsys/Subsystem.hpp>
#include <cstdint>

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

namespace {
    /** capacity for the thread-local scratch arena behind tostr() and
     *  TempFlatSink.  Fixed on purpose: a test that outgrows it is news
     **/
    constexpr std::uint32_t c_temp_arena_capacity = 64 * 1024;

    using UtestAppConfig  = xo::AppConfig<xo::S_indentlog2_tag, xo::S_facet_tag>;
    using UtestAppContext = xo::AppContext<xo::S_indentlog2_tag, xo::S_facet_tag>;

    /** the one context chain for this executable: see object2_utest_appcx.hpp **/
    UtestAppContext & utest_appcx() {
        static UtestAppContext s_cx{
            UtestAppConfig(xo::Indentlog2Config(xo::pp::PpConfig::colored(),
                                                c_temp_arena_capacity),
                           xo::FacetConfig::make_default()) };

        return s_cx;
    }
}

namespace xo {
    FacetAppcx &
    object2_utest_facet_appcx()
    {
        return utest_appcx().cx<S_facet_tag>();
    }
} /*namespace xo*/

int
main(int argc, char* argv[])
{
    using xo::Subsystem;

    /* the whole chain, before any test: fixes the scratch arena's capacity
     * before its first use
     */
    (void)utest_appcx();

    Subsystem::initialize_all();

    // Run Catch2's test session
    int result = Catch::Session().run(argc, argv);

    return result;
}

/* end object2_utest_main.cpp */
