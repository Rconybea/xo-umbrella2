/** @file X1CollectorConfig.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "object_age.hpp"
#include "generation.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <array>
#include <cstdint>

namespace xo {
    namespace mm {
        struct X1CollectorConfig {
            using size_type = std::size_t;

            /** copy of this config,
             *  with @c arena_config_.size_ set to @p gen_z
             **/
            X1CollectorConfig with_size(std::size_t gen_z);

            generation age2gen(object_age age) const noexcept {
                return generation(age % n_survive_threshold_);
            }

        public:
            // ----- Instance Variables -----

            /** optional name, for diagnostics **/
            std::string name_;

            /** Configuration for collector spaces.
             *  Will have at least {nursery,tenured} x {from,to} spaces.
             *  Not using name_ member.
             *
             *  REQUIRE:
             *  - arena_config_.store_header_flag_ must be true
             **/
            ArenaConfig arena_config_;

            /** storage for N object types requires 8*N bytes **/
            std::size_t object_types_z_ = 2*1024*1024;

            /** storage for N object roots requires 8*N bytes **/
            std::size_t object_roots_z_ = 16*1024;

            /** number of bits to represent generation **/
            std::uint64_t gen_bits_ = 8;

            /** number of bits to represent tseq **/
            std::uint64_t tseq_bits_ = 24;

            /** Number of generations.
             *  Must be at least 2.
             **/
            uint32_t n_generation_ = 2;

            /** Number of promotion steps.
             *  An object that survives this number of collections
             *  advances to the next generation.
             **/
            uint32_t n_survive_threshold_ = 2;

            /** Trigger garbage collection when to-space allocation for
             *  generation g reaches gc_trigger_v_[g]
             **/
            std::array<size_type, c_max_generation> gc_trigger_v_;

            /** true -> enable incremental collection.
             *  false -> only do full collection.
             *
             *  Incremental collection requires mutation logs.
             **/
            bool allow_incremental_gc_ = true;

            /** If non-zero remember statistics for
             *  the last @p stats_history_z_ collections.
             **/
            uint32_t stats_history_z_ = false;

            /** true to enable debug logging **/
            bool debug_flag_ = false;
        };

        
    } /*namespace mm*/
} /*namespace xo*/

/* end X1CollectorConfig.hpp */

