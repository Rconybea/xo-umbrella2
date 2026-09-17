/** @file FacetAppcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/FacetAppcx.hpp"

namespace xo {

    constexpr uint64_t c_facetappcx_creation_secret = 0x1234;

    FacetAppcx::FacetAppcx(const FacetConfig & cfg, const Indentlog2Appcx & ilog2_appcx)
      : init_evidence_{InitSubsys<S_facet_tag>::require()},
        facet_evp_{c_facetappcx_creation_secret},
        indentlog2appcx_creation_evidence_{ilog2_appcx.creation_evidence()},
        config_{cfg},
        facet_registry_{FacetRegistry::instance(cfg.facet_registry_capacity())},
        type_registry_{TypeRegistry::instance(cfg.type_registry_capacity())},
        indentlog2_appcx_{ilog2_appcx}
    {}

    Indentlog2Appcx::CreationEvidence
    FacetAppcx::indentlog2appcx_creation_evidence() const
    {
        return indentlog2appcx_creation_evidence_;
    }

} /*namespace xo*/

/* end FacetAppcx.cpp */
