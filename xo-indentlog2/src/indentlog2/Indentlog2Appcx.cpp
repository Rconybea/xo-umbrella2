/** @file Indentlog2Appcx.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "cx/Indentlog2Appcx.hpp"

namespace xo {

    Indentlog2Appcx::Indentlog2Appcx(const Indentlog2Config & cfg)
    : init_evidence_{InitSubsys<S_indentlog2_tag>::require()},
      config_{cfg},
      sink_factory_{cfg.pp_config()}
    {
        using xo::pp::SinkFactory;
        using xo::pp::ThreadLogState;

        // Upgrade to pretty-printing logger in new threads.
        SinkFactory::set_instance(&sink_factory_);

        // Upgrade to pretty-printing logger for calling (main) thread.
        ThreadLogState::log_set_sink(SinkFactory::instance().create());

        // scratch arena (per thread)
        TempArena::init(config_.temp_arena_capacity());

        // scratch pretty-printing (for conversion to string)
        TempPrettySink::init(cfg.pp_config());
    }
} /*namespace xo*/

/* end Indentlog2Appcx.cpp */
