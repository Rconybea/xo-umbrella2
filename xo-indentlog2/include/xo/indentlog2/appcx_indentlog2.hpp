/** @file appcx_indentlog2.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include "config_indentlog2.hpp"
#include "TempArena.hpp"
#include <xo/subsys/AppContext.hpp>

namespace xo {
    /** There's some configuration that logically belongs to indentlog2/,
     *  but is stored in thread-local storage.
     *
     *  This is _not_ a model to use as a general-purpose pattern.
     **/
    class Indentlog2_Appcx {
    public:
        using TempArena = xo::mm::TempArena;

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
            : config_{cfg}
            {
                TempArena::init(config_.temp_arena_capacity());
            }

        TempArena & temp_arena() { return temp_arena_; }

    private:
        /** xo-indentlog2/ configuration **/
        Indentlog2_Config config_;

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
