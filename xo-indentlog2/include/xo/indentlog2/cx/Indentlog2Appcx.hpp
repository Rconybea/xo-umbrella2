/** @file Indentlog2Appcx.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include "Indentlog2Config.hpp"
#include "xo/indentlog2/TempArena.hpp"
#include "xo/indentlog2/print/PrettySinkFactory.hpp"
#include "xo/indentlog2/print/PrettySink.hpp"
#include "xo/ppsink/TempPpSink.hpp"
#include <xo/subsys/AppContext.hpp>
#include <concepts>
#include <xo/ppsink/LogState.hpp>

namespace xo {
    /** There's some configuration that logically belongs to indentlog2/,
     *  but is stored in thread-local storage.
     *
     *  This is _not_ a model to use as a general-purpose pattern.
     **/
    class Indentlog2Appcx {
    public:
        using TempPpSink = xo::pp::TempPpSink;
        using TempArena = xo::mm::TempArena;
        using PrettySinkFactory = xo::pp::PrettySinkFactory;
        using PrettySink = xo::pp::PrettySink;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;
        using DArena = xo::mm::DArena;

    public:
        /** non-template initialization, from @p cfg.
         *
         *  Primary driver for indentlog2 init.
         **/
        Indentlog2Appcx(const Indentlog2Config & cfg);

        /** Template for indentlog2 init. Works with AppConfig, AppContext.
         *
         *  @p deps  contexts of the subsystems below this one.  Unused:
         *            xo-indentlog2 is at the bottom of the tower.
         *  @p cfg   configuration for this subsystem
         **/
        template <typename Deps>
        Indentlog2Appcx(Deps & /*deps*/,
                        const Indentlog2Config & cfg) : Indentlog2Appcx(cfg) {}

        InitEvidence init_evidence() const { return init_evidence_; }
        const Indentlog2Config & config() const { return config_; }
        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *  Only for pools owned by current thread.
         **/
        void visit_pools(const MemorySizeVisitor & fn) const;

        TempArena & temp_arena() { return temp_arena_; }
        TempPpSink & temp_ppsink() { return temp_ppsink_; }

    private:
        /** ensures low-level subsystem initialization **/
        InitEvidence init_evidence_;

        /** xo-indentlog2/ configuration **/
        Indentlog2Config config_;

        /** create pretty-printing sinks, connected to clog
         *  (typically one per thread)
         **/
        PrettySinkFactory sink_factory_;

        /** temporary arena storage **/
        TempArena temp_arena_;

        /** temporary pretty sink **/
        TempPpSink temp_ppsink_;
    };

    template <>
    class SubsystemContext<S_indentlog2_tag> {
    public:
        using Type = Indentlog2Appcx;
    };
    /** @brief types that can attest xo-indentlog2 has been configured.
     *
     *  An Indentlog2Appcx can only be constructed by configuring the
     *  subsystem, so a value that can hand one over is evidence the
     *  configuration happened -- notably that TempPrettySink::init() ran, which
     *  everything reaching for a scratch sink depends on.
     *
     *  Use it to state a dependency that would otherwise live in a comment:
     *
     *    static_assert(carries_indentlog2<HFloat>);
     *
     *  NB this checks that the TYPE can produce the witness.  It is only proof
     *  because the witness itself is unforgeable; see InitEvidence.
     **/
    template <typename T>
    concept carries_indentlog2 = requires (const T & x) {
        { x.indentlog2_appcx() } -> std::convertible_to<const Indentlog2Appcx &>;
    };

} /*namespace xo*/

/* end Indentlog2Appcx.hpp */
