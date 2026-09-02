/** @file FacetConfig.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "init_facet.hpp"
#include <xo/subsys/AppContext.hpp>
#include <cstdint>

namespace xo {
    /** @brief configuration for subsystem xo-facet/ **/
    class FacetConfig {
    public:
        explicit FacetConfig(uint32_t facet_cap,
                             uint32_t type_cap) : init_evidence_{InitSubsys<S_facet_tag>::require()},
                                                  facet_registry_capacity_{facet_cap},
                                                  type_registry_capacity_{type_cap} {}

        uint32_t type_registry_capacity() const { return type_registry_capacity_; }
        uint32_t facet_registry_capacity() const { return facet_registry_capacity_; }

    private:
        /** ensures low-level subsystem initialization **/
        InitEvidence init_evidence_;

        /** max capacity for facet registry (FacetRegistry::instance()) **/
        uint32_t facet_registry_capacity_;

        /** max capacity for type registry (TypeRegistry::instance()) **/
        uint32_t type_registry_capacity_;
    };

    /** xo-facet contributes both a configuration and a context **/
    template <>
    class SubsystemConfig<S_facet_tag> {
    public:
        using Type = FacetConfig;
    };
} /*namespace xo*/

/* end FacetConfig.hpp */
