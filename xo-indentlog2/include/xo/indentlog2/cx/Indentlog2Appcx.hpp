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
#include <xo/subsys/Evidence.hpp>
#include <concepts>
#include <xo/ppsink/LogState.hpp>

namespace xo {
    /** NOTE: there's some configuration that logically belongs to indentlog2/,
     *  but is stored in thread-local storage.
     *
     *  This is _not_ a model to use as a general-purpose pattern.
     **/
    class Indentlog2Appcx {
    public:
        using CreationEvidence = Evidence<class Indentlog2AppcxCreated_tag>;
        using CreationEvp = EvidenceProvider<CreationEvidence>;
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
        CreationEvidence creation_evidence() const { return indentlog2_evp_; }
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

        /** provides evidence that Indentlog2Appcx has been created **/
        CreationEvp indentlog2_evp_;

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
     *  Satisfied by Indentlog2Appcx itself
     *  + anything that can produce Indentlog2Appcx::creation_evidence().
     **/
    template <typename T>
    concept carries_indentlog2_appcx = requires (const T & x) {
        { x.indentlog2appcx_creation_evidence() } -> std::convertible_to<Indentlog2Appcx::CreationEvidence>;
    };

} /*namespace xo*/

/* end Indentlog2Appcx.hpp */
