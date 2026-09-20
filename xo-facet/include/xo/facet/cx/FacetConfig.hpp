/** @file FacetConfig.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/facet/init_facet.hpp"
#include <xo/subsys/AppContext.hpp>
#include <cstdint>

namespace xo {
    /** @brief configuration for subsystem xo-facet/ **/
    class FacetConfig {
    public:
        explicit FacetConfig(uint32_t facet_cap,
                             uint32_t type_cap,
                             uint32_t storage_base_align);

        /** default configuration **/
        static FacetConfig make_default();

        uint32_t type_registry_capacity() const { return type_registry_capacity_; }
        uint32_t facet_registry_capacity() const { return facet_registry_capacity_; }
        uint32_t storage_base_align() const { return storage_base_align_; }

    private:
        /** max capacity for facet registry (FacetRegistry::instance()) **/
        uint32_t facet_registry_capacity_ = 0;

        /** max capacity for type registry (TypeRegistry::instance()) **/
        uint32_t type_registry_capacity_ = 0;

        /** base alignment for handle storage (see AllocFlywheel, DHandleStore) **/
        uint32_t storage_base_align_ = 0;
    };

    /** xo-facet contributes both a configuration and a context **/
    template <>
    class SubsystemConfig<S_facet_tag> {
    public:
        using Type = FacetConfig;
    };
} /*namespace xo*/

/* end FacetConfig.hpp */
