/** @file appcx_indentlog2.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include "config_indentlog2.hpp"
#include "TempArena.hpp"
#include "print/PrettySink.hpp"
#include <xo/subsys/AppContext.hpp>
#include <xo/ppsink/LogState.hpp>

namespace xo {
    /** There's some configuration that logically belongs to indentlog2/,
     *  but is stored in thread-local storage.
     *
     *  This is _not_ a model to use as a general-purpose pattern.
     **/
    class Indentlog2_Appcx {
    public:
        using TempArena = xo::mm::TempArena;
        using PrettySinkFactory = xo::pp::PrettySinkFactory;
        using PrettySink = xo::pp::PrettySink;

    public:
        /** @p deps  contexts of the subsystems below this one.  Unused:
         *            xo-indentlog2 is at the bottom of the tower.
         *  @p cfg   configuration for this subsystem
         *
         *  Constructing this context IS xo-indentlog2's initialization -- it
         *  sizes the thread-local scratch arena from @p cfg.
         **/
        template <typename Deps>
        Indentlog2_Appcx(Deps & /*deps*/, const Indentlog2_Config & cfg)
            : config_{cfg},
              sink_factory_{cfg.pp_config()}
            {
                using xo::pp::SinkFactory;
                using xo::pp::ThreadLogState;

                // Upgrade to pretty-printing logger in new threads.
                SinkFactory::set_instance(&sink_factory_);

                // Upgrade to pretty-printing logger for calling (main) thread.
                ThreadLogState::log_set_sink(SinkFactory::instance().create());

                TempArena::init(config_.temp_arena_capacity());
            }

        TempArena & temp_arena() { return temp_arena_; }

    private:
        /** xo-indentlog2/ configuration **/
        Indentlog2_Config config_;

        /** create pretty-printing sinks, connected to clog
         *  (typically one per thread)
         **/
        PrettySinkFactory sink_factory_;

        /** temporary arena storage **/
        TempArena temp_arena_;
    };

    template <>
    class SubsystemContext<S_indentlog2_tag> {
    public:
        using Type = Indentlog2_Appcx;
    };
} /*namespace xo*/

/* end appcx_indentlog2.hpp */
