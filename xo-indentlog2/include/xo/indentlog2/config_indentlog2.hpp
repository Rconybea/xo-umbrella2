/** @file config_indentlog2.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include "init_indentlog2.hpp"
#include "print/PpConfig.hpp"
#include <xo/subsys/AppContext.hpp>
#include <cstdint>

namespace xo {
    /** @brief configuration for subsystem xo-indentlog2/ **/
    class Indentlog2_Config {
    public:
        using PpConfig = xo::pp::PpConfig;

    public:
        Indentlog2_Config(const PpConfig & ppconfig, uint32_t cap);

        const PpConfig & pp_config() const { return pp_config_; }
        uint32_t temp_arena_capacity() const { return temp_arena_capacity_; }

    private:
        /** ensures low-level subsystem initialization **/
        InitEvidence init_evidence_;

        /** configure default pretty-printer **/
        PpConfig pp_config_;

        /** capacity for thread-local temporary arena (TempArena::local()) **/
        uint32_t temp_arena_capacity_;
    };

    /** xo-indentlog2 contributes both a configuration and a context **/
    template <>
    class SubsystemConfig<S_indentlog2_tag> {
    public:
        using Type = Indentlog2_Config;
    };

} /*namespace xo*/

/* end config_indentlog2.hpp */
