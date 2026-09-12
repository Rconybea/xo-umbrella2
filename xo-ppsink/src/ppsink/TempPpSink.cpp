/** @file TempPpSink.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "TempPpSink.hpp"
#include "PpSinkFactory.hpp"
#include "FlatSink.hpp"
#include <cassert>

namespace xo::pp {
    using ColorSelect = PpSinkFactory::ColorSelect;
    using SinkOutput = PpSinkFactory::SinkOutput;

    thread_local std::unique_ptr<PpSink>
    TempPpSink::s_ppsink;

    thread_local PpSinkFactory *
    TempPpSink::s_factory = nullptr;

    void
    TempPpSink::reset_local(std::unique_ptr<PpSink> pps)
    {
        /* attribute the supplied sink to the factory in force now, so
         * local() treats it as current rather than rebuilding over it
         */
        s_factory = (pps ? &PpSinkFactory::instance() : nullptr);
        s_ppsink = std::move(pps);
    }

    PpSink *
    TempPpSink::check_local()
    {
        return s_ppsink.get();
    }

    bool
    TempPpSink::is_flat()
    {
        auto pps = s_ppsink.get();

        return (pps && dynamic_cast<FlatSink *>(pps));
    }

    PpSink &
    TempPpSink::local()
    {
        auto & f = PpSinkFactory::instance();

        /* rebuild when the factory changed under us: the cached sink belongs
         * to a generation this thread has outlived.  See TempPpSink.hpp.
         */
        if (!s_ppsink || (s_factory != &f)) {
            auto pps = f.create(ColorSelect::k_plain,
                                SinkOutput::k_memory);

            s_ppsink = std::move(pps);
            s_factory = &f;
        }

        assert(s_ppsink);

        return *s_ppsink;
    }

} /*namespace xo::pp*/

/* end TempPpSink.cpp */
