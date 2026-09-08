/** @file FacetAppcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "FacetConfig.hpp"
#include "xo/facet/FacetRegistry.hpp"
#include "xo/facet/TypeRegistry.hpp"
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <xo/subsys/AppContext.hpp>
#include <concepts>

namespace xo {
    /** @brief application context for the xo-facet subsystem.
     **/
    class FacetAppcx {
    public:
        using FacetRegistry = xo::facet::FacetRegistry;
        using TypeRegistry = xo::facet::TypeRegistry;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

    public:
        /** non-template initialization, from @p cfg.
         *  Primary driver for facet init.
         *  @p ilog2_appcx. Not used directly; proof of work.
         **/
        FacetAppcx(const FacetConfig & cfg,
                   const Indentlog2Appcx & ilog2_appcx);

        /** Template for facet init.  Works with AppConfig, AppContext.
         *
         *  @p deps contexts for the subsystems below this one.
         *  @p cfg  configuration for this subsystem.
         **/
        template <typename Deps>
        FacetAppcx(Deps & deps,
                   const FacetConfig & cfg) : FacetAppcx(cfg, deps.template cx<S_indentlog2_tag>()) {}

        const Indentlog2Appcx & indentlog2_appcx() const { return indentlog2_appcx_; }

        InitEvidence init_evidence() const { return init_evidence_; }
        const FacetConfig & config() const { return config_; }
        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *
         *  Two pools: the facet registry, then the type registry.
         *
         *  This subsystem's pools ONLY -- it does not descend into
         *  @ref indentlog2_appcx_.  A caller that wants the whole stack walks
         *  it explicitly:
         *
         *    cx.visit_pools(fn);
         *    cx.indentlog2_appcx().visit_pools(fn);
         *
         *  Descending automatically would double-count for any caller that
         *  already walks the chain, and there is no way for the visitor to tell
         *  which subsystem a pool came from.
         **/
        void visit_pools(const MemorySizeVisitor & fn) const {
            facet_registry_.visit_pools(fn);
            type_registry_.visit_pools(fn);
        }

        FacetRegistry & facet_registry() { return facet_registry_; }
        TypeRegistry & type_registry() { return type_registry_; }

    private:
        /** ensures low-level subsystem initialization **/
        InitEvidence init_evidence_;

        /** xo-facet/ configuration **/
        FacetConfig config_;

        /** cross-facet conversion table **/
        FacetRegistry & facet_registry_;

        /** type table **/
        TypeRegistry & type_registry_;

        /** xo-indentlog2/ context **/
        const Indentlog2Appcx & indentlog2_appcx_;
    };

    /** @brief types that can attest xo-facet has been configured.
     *
     *  Satisfied by FacetAppcx itself, by AllocFlywheel (which retains the
     *  context it was made with), and by any DObjectHandle (which reaches it
     *  through its flywheel).  So "I hold one of these" and "xo-facet was
     *  configured" are the same statement, checkably.
     **/
    template <typename T>
    concept carries_facet_appcx = requires (const T & x) {
        { x.facet_appcx() } -> std::convertible_to<const FacetAppcx &>;
    };

    template <>
    class SubsystemContext<S_facet_tag> {
    public:
        using Type = FacetAppcx;
    };
}

/* end FacetAppcx.hpp */
