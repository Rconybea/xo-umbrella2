/** @file reactor2websock_utest_main.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

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

    auto app = xo::UtestAppStart("utest.reactor2websock");

    int retval = app.init(argc, argv);
    if (retval)
        return retval;

    app.setup();

    return app.run();
}

/* end reactor2websock_utest_main.cpp */
