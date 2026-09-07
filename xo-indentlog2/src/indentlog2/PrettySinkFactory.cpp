/** @file PrettySinkFactory.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "print/PrettySinkFactory.hpp"
#include <iostream>

namespace xo::pp {

    std::unique_ptr<PpSink>
    PrettySinkFactory::create()
    {
        return std::make_unique<PrettySink>(pp_config_,
                                            std::clog.rdbuf());
    }

} /*namespace xo::pp*/

/* end PrettySinkFactory.cpp */
