/** @file Indentlog2Config.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include "xo/indentlog2/init_indentlog2.hpp"
#include "xo/indentlog2/print/PpConfig.hpp"
#include <xo/subsys/AppContext.hpp>
#include <cstdint>

namespace xo {
    /** @brief configuration for subsystem xo-indentlog2/ **/
    class Indentlog2Config {
    public:
        using PpConfig = xo::pp::PpConfig;

    public:
        Indentlog2Config(const PpConfig & ppconfig, uint32_t cap);

        /** default configuration **/
        static Indentlog2Config make_default();

        const PpConfig & pp_config() const { return pp_config_; }
        uint32_t temp_arena_capacity() const { return temp_arena_capacity_; }

    private:
        /** configure default pretty-printer **/
        PpConfig pp_config_;

        /** capacity for thread-local temporary arena (TempArena::local()) **/
        uint32_t temp_arena_capacity_;
    };

    /** xo-indentlog2 contributes both a configuration and a context **/
    template <>
    class SubsystemConfig<S_indentlog2_tag> {
    public:
        using Type = Indentlog2Config;
    };

} /*namespace xo*/

/* end Indentlog2Config.hpp */
