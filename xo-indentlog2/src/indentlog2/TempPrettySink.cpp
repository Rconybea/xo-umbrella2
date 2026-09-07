/** @file TempPrettySink.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "TempPrettySink.hpp"
#include <cassert>

namespace xo::pp {
    PpConfig
    TempPrettySink::s_ppconfig;

    void
    TempPrettySink::init(const PpConfig & cfg)
    {
        s_ppconfig = cfg;
    }

    PrettySink &
    TempPrettySink::local()
    {
        /** allocate temp pretty sink per thread **/
        static thread_local PrettySink * s_ppsink = nullptr;

        if (!s_ppsink) {
            s_ppsink = new PrettySink(s_ppconfig,
                                      nullptr /*out*/);
        }

        assert(s_ppsink);

        return *s_ppsink;
    }

} /*namespace xo::pp*/

/* end TempPrettySink.cpp */
