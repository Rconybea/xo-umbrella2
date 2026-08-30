/** @file config_indentlog2.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include <cstdint>

namespace xo {
    /** @brief configuration for subsystem xo-indentlog2/ **/
    class Indentlog2_Config {
    public:
        Indentlog2_Config(uint32_t cap) : temp_arena_capacity_{cap} {}

        uint32_t temp_arena_capacity() const { return temp_arena_capacity_; }

    private:
        /** capacity for thread-local temporary arena (TempArena::local()) **/
        uint32_t temp_arena_capacity_ = 4 * 1024;
    };
} /*namespace xo*/

/* end config_indentlog2.hpp */
