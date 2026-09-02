/** @file FacetAppcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "FacetConfig.hpp"
#include "FacetRegistry.hpp"
#include <xo/indentlog2/appcx_indentlog2.hpp>
#include <xo/subsys/AppContext.hpp>

namespace xo {
    /** @brief application context for the xo-facet subsystem.
     **/
    class FacetAppcx {
    public:
        using FacetRegistry = xo::facet::FacetRegistry;

    public:
        template <typename Deps>
        FacetAppcx(Deps & /*deps*/,
                   const FacetConfig & cfg) : config_{cfg},
                                              registry_{FacetRegistry::instance(cfg.facet_registry_capacity())}
        {}

        FacetRegistry & registry() { return registry_; }

    private:
        /** xo-facet/ configuration **/
        FacetConfig config_;

        /** cross-facet conversion table **/
        FacetRegistry & registry_;
    };

    template <>
    class SubsystemContext<S_facet_tag> {
    public:
        using Type = FacetAppcx;
    };
}

/* end FacetAppcx.hpp */
