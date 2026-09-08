/** @file TempPrettySink.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "TempPrettySink.hpp"
#include <cassert>

namespace xo::pp {
    PpConfig
    TempPrettySink::s_ppconfig;

    thread_local std::unique_ptr<PrettySink>
    TempPrettySink::s_ppsink;

    void
    TempPrettySink::init(const PpConfig & cfg)
    {
        s_ppconfig = cfg;
    }

    PrettySink *
    TempPrettySink::check_local()
    {
        return s_ppsink.get();
    }

    PrettySink &
    TempPrettySink::local()
    {
        if (!s_ppsink) {
            s_ppsink.reset(new PrettySink(s_ppconfig,
                                          nullptr /*out*/));
        }

        assert(s_ppsink);

        return *s_ppsink;
    }

} /*namespace xo::pp*/

/* end TempPrettySink.cpp */
