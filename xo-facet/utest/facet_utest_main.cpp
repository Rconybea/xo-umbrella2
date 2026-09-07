/* file facet_utest_main.cpp */

#define CATCH_CONFIG_EXTERNAL_INTERFACES // before UtestListener.hpp

#include "FacetUtestAppcx.hpp"
#include "xo/facet/init_facet.hpp"
#include "xo/facet/cx/FacetConfig.hpp"
#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/indentlog2/cx/Indentlog2Config.hpp>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/testutil/UtestAppStart.hpp>
#include <xo/testutil/UtestListener.hpp>

namespace xo {
    CATCH_REGISTER_LISTENER(UtestListener);
}

namespace {
    using std::uint32_t;

    /** capacity for facet interface registry **/
    constexpr uint32_t c_facet_registry_capacity = 1024;
    /** capacity for type registry **/
    constexpr uint32_t c_type_registry_capacity = 1024;
    /** capacity for thread-local scratch arena behind tostr()/toppstr() **/
    constexpr uint32_t c_temp_arena_capacity = 64 * 1024;
}

int
main(int argc, char* argv[])
{
    using xo::FacetUtestAppcx;
    using xo::FacetConfig;
    using xo::Indentlog2Config;

    using xo::mm::ArenaConfig;
    using xo::pp::PpConfig;
    using std::clog;

    auto app = xo::UtestAppStart("utest.facet");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

    using UtestAppConfig = FacetUtestAppcx::UtestAppConfig;

    UtestAppConfig utest_cfg{
        Indentlog2Config(PpConfig().with_logbuf_config
                             (ArenaConfig().with_size(1024 * 1024)),
                         c_temp_arena_capacity),
        FacetConfig(c_facet_registry_capacity,
                    c_type_registry_capacity)};

    FacetUtestAppcx::configure(utest_cfg);

    app.setup(); // calls Subsystem::initialize_all()

    return app.run();
}

/* end facet_utest_main.cpp */
