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
        explicit FacetConfig(uint32_t cap) : init_evidence_{InitSubsys<S_facet_tag>::require()},
                                             facet_registry_capacity_{cap} {}

        uint32_t facet_registry_capacity() const { return facet_registry_capacity_; }

    private:
        /** ensures low-level subsystem initialization **/
        InitEvidence init_evidence_;

        /** max capacity for facet registry (FacetRegistry::instance()) **/
        uint32_t facet_registry_capacity_;
    };

    /** xo-facet contributes both a configuration and a context **/
    template <>
    class SubsystemConfig<S_facet_tag> {
    public:
        using Type = FacetConfig;
    };
} /*namespace xo*/

/* end FacetConfig.hpp */
