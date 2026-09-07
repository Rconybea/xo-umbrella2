/** @file FacetConfig.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/FacetConfig.hpp"

namespace xo {

    FacetConfig::FacetConfig(uint32_t facet_cap,
                             uint32_t type_cap)
      : facet_registry_capacity_{facet_cap},
        type_registry_capacity_{type_cap}
    {}

    FacetConfig
    FacetConfig::make_default()
    {
        return FacetConfig(1024, 1024);
    }

} /*namespace xo*/

/* end FacetConfig.cpp */
