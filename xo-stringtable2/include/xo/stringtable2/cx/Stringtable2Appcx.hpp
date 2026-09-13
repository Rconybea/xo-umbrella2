/** @file Stringtable2Appcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "Stringtable2Config.hpp"
#include <xo/facet/cx/FacetAppcx.hpp>

namespace xo {
    /** @brief Application-level state for the stringtable2 subsystem
     **/
    class Stringtable2Appcx {
    public:
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

    public:
        /** non-template initialization, from @p cfg.
         *
         *  @p facet_appcx.  The registry this subsystem's facet
         *  implementations register with.  Not used directly yet; proof of
         *  work, and the seam for appcx-configured FacetRegistry.
         **/
        Stringtable2Appcx(const Stringtable2Config & cfg,
                          const FacetAppcx & facet_appcx);

        /** Template for stringtable2 init.  Works with AppConfig, AppContext.
         *
         *  @p deps  contexts of the subsystems below this one.
         *  @p cfg   configuration for this subsystem.
         **/
        template <typename Deps>
        Stringtable2Appcx(Deps & deps,
                          const Stringtable2Config & cfg)
            : Stringtable2Appcx(cfg, deps.template cx<S_facet_tag>()) {}

        InitEvidence init_evidence() const { return init_evidence_; }
        const Stringtable2Config & config() const { return config_; }
        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *  (probably always noop)
         **/
        void visit_pools(const MemorySizeVisitor &) const {}

    private:
        /** ensure this subsystem's initialization -- which is what registers
         *  its facet implementations
         **/
        InitEvidence init_evidence_;

        /** xo-stringtable2/ configuration **/
        Stringtable2Config config_;

        /** xo-facet/ context **/
        const FacetAppcx & facet_appcx_;
    };

    template <>
    class SubsystemContext<S_stringtable2_tag> {
    public:
        using Type = Stringtable2Appcx;
    };
} /*namespace xo*/

/* end Stringtable2Appcx.hpp */
