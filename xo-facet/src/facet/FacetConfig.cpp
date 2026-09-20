/** @file FacetConfig.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/FacetConfig.hpp"

namespace xo {

    FacetConfig::FacetConfig(uint32_t facet_cap,
                             uint32_t type_cap,
                             uint32_t storage_base_align)
      : facet_registry_capacity_{facet_cap},
        type_registry_capacity_{type_cap},
        storage_base_align_{storage_base_align}
    {}

    FacetConfig
    FacetConfig::make_default()
    {
        uint32_t facet_cap = 1024;
        uint32_t type_cap = 1024;
        uint32_t storage_base_align = 1024 * 1024;

        return FacetConfig(facet_cap,
                           type_cap,
                           storage_base_align);
    }

} /*namespace xo*/

/* end FacetConfig.cpp */
