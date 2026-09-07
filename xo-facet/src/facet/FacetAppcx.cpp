/** @file FacetAppcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/FacetAppcx.hpp"

namespace xo {

    FacetAppcx::FacetAppcx(const FacetConfig & cfg, const Indentlog2Appcx & ilog2_appcx)
      : init_evidence_{InitSubsys<S_facet_tag>::require()},
        config_{cfg},
        facet_registry_{FacetRegistry::instance(cfg.facet_registry_capacity())},
        type_registry_{TypeRegistry::instance(cfg.type_registry_capacity())},
        indentlog2_appcx_{ilog2_appcx}
    {}

} /*namespace xo*/

/* end FacetAppcx.cpp */
