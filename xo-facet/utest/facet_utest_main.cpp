/* file facet_utest_main.cpp */

#define CATCH_CONFIG_EXTERNAL_INTERFACES // before UtestListener.hpp

#include <xo/facet/init_facet.hpp>
#include <xo/facet/cx/FacetAppcx.hpp>
#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/indentlog2/appcx_indentlog2.hpp>
#include <xo/indentlog2/config_indentlog2.hpp>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/subsys/AppContext.hpp>
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
    using xo::S_facet_tag;
    using xo::S_indentlog2_tag;
    using xo::FacetConfig;
    using xo::Indentlog2_Config;

    using xo::AppContext;
    using xo::AppConfig;
    using xo::mm::ArenaConfig;
    //using xo::pp::ThreadPrettySink;
    using xo::pp::PpConfig;
    using std::clog;

    using UtestAppConfig = AppConfig<S_indentlog2_tag, S_facet_tag>;
    using UtestAppContext = AppContext<S_indentlog2_tag, S_facet_tag>;

    auto app = xo::UtestAppStart("utest.facet");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

#ifdef OBSOLETE
    {
        /* setup pretty-printing */
        ThreadPrettySink::thread_install_once(PpConfig().with_logbuf_config
                                                  (ArenaConfig().with_size(1024*1024)),
                                              clog.rdbuf());
    }
#endif

    UtestAppConfig utest_config{
        Indentlog2_Config(PpConfig().with_logbuf_config
                          (ArenaConfig().with_size(1024 * 1024)),
                          c_temp_arena_capacity),
        FacetConfig(c_facet_registry_capacity,
                    c_type_registry_capacity)};
    UtestAppContext utest_appcx{utest_config};

    app.setup(); // calls Subsystem::initialize_all()

    return app.run();
}

/* end facet_utest_main.cpp */
