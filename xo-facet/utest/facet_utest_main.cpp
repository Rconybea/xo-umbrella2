/* file facet_utest_main.cpp */

#define CATCH_CONFIG_EXTERNAL_INTERFACES // before UtestListener.hpp

#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/testutil/UtestAppStart.hpp>
#include <xo/testutil/UtestListener.hpp>

namespace xo {
    CATCH_REGISTER_LISTENER(UtestListener);
}

int
main(int argc, char* argv[])
{
    using xo::pp::ThreadPrettySink;
    using xo::pp::PpConfig;
    using xo::mm::ArenaConfig;
    using std::clog;

    auto app = xo::UtestAppStart("utest.facet");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

    {
        /* setup pretty-printing */
        ThreadPrettySink::thread_install_once(PpConfig().with_logbuf_config
                                                  (ArenaConfig().with_size(1024*1024)),
                                              clog.rdbuf());
    }

    app.setup(); // calls Subsystem::initialize_all()

    return app.run();
}

/* end facet_utest_main.cpp */
