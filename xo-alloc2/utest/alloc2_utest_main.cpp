/* file alloc2_utest_main.cpp */

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

#include <xo/facet/init_facet.hpp>
#include <xo/facet/cx/FacetAppcx.hpp>
#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/indentlog2/appcx_indentlog2.hpp>
#include <xo/indentlog2/config_indentlog2.hpp>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/testutil/UtestAppStart.hpp>
#include <xo/testutil/UtestListener.hpp>

namespace xo {
    CATCH_REGISTER_LISTENER(UtestListener);
}

int
main(int argc, char* argv[])
{
    //using xo::pp::ThreadPrettySink;
    using xo::S_facet_tag;
    using xo::S_indentlog2_tag;
    using xo::FacetConfig;
    using xo::Indentlog2_Config;
    using xo::pp::PpConfig;
    using xo::AppContext;
    using xo::AppConfig;
    using xo::mm::ArenaConfig;
    using std::clog;

    using UtestAppConfig = AppConfig<S_indentlog2_tag, S_facet_tag>;
    using UtestAppContext = AppContext<S_indentlog2_tag, S_facet_tag>;

    auto app = xo::UtestAppStart("utest.alloc2");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

#ifdef OBSOLETE
    {
        // setup pretty-printing

        ThreadPrettySink::thread_install_once(PpConfig().with_logbuf_config
                                                  (ArenaConfig().with_size(1024*1024)),
                                              clog.rdbuf());
    }
#endif

    UtestAppConfig utest_config{
        Indentlog2_Config(PpConfig().with_logbuf_config
                          (ArenaConfig().with_size(1024 * 1024)),
                          64 * 1024 /*c_temp_arena_capacity*/),
        FacetConfig(1024 /*c_facet_registry_capacity*/,
                    1024 /*c_type_registry_capacity*/)};
    UtestAppContext utest_appcx{utest_config};

    app.setup(); // calls Subsystem::initialize_all()

    return app.run();
}

/* end alloc2_utest_main.cpp */
