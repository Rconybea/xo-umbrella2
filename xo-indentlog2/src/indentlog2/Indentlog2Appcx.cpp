/** @file Indentlog2Appcx.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "cx/Indentlog2Appcx.hpp"
#include "print/PrettySink.hpp"

namespace xo {
    using xo::pp::PpSinkFactory;
    using xo::pp::PpSink;
    using xo::pp::ThreadLogState;

    Indentlog2Appcx::Indentlog2Appcx(const Indentlog2Config & cfg)
    : init_evidence_{InitSubsys<S_indentlog2_tag>::require()},
      config_{cfg},
      sink_factory_{cfg.pp_config()}
    {
        using ColorSelect = PpSinkFactory::ColorSelect;
        using SinkOutput = PpSinkFactory::SinkOutput;

        // Upgrade to pretty-printing logger in new threads.
        // Also drives behavior of TempPpSink instances
        //
        // PpSinkFactory instance is global singleton;
        // each instance is immutable.
        //
        PpSinkFactory::set_instance(&sink_factory_);

        {
            auto & f = PpSinkFactory::instance();
            auto pps = f.create(ColorSelect::k_colored,
                                SinkOutput::k_clog);

            // Upgrade to pretty-printing logger for calling (main) thread.
            ThreadLogState::log_set_sink(std::move(pps));
        }

        // scratch arena (per thread)
        TempArena::init(config_.temp_arena_capacity());

        // We don't have to initialze TempPpSink, since it's lazy.
        // Mentioning here because visit_pools() reaches it.
    }

    void
    Indentlog2Appcx::visit_pools(const MemorySizeVisitor & fn) const
    {
        if (const DArena * arena = TempArena::check_local())
            arena->visit_pools(fn);

        if (const PpSink * sink = TempPpSink::check_local()) {
            // dynamic cast necessary.
            // PpSink doesn't support visit_pools(), since MemorySizeInfo
            // is at higher level, in xo-arena

            auto ppsink
                = dynamic_cast<const PrettySink *>(sink);

            if (ppsink)
                ppsink->visit_pools(fn);
        }
    }

} /*namespace xo*/

/* end Indentlog2Appcx.cpp */
