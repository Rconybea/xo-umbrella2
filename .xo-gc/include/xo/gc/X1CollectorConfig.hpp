/** @file X1CollectorConfig.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "GCObjectStoreConfig.hpp"
#include "MutationLogConfig.hpp"
#include "object_age.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <array>
#include <cstdint>

namespace xo {
    namespace mm {
        /** @brief configuration for X1 collector
         **/
        struct X1CollectorConfig {
            using size_type = std::size_t;

            /** copy of this config,
             *  but with @ref name_ set to @p name
             **/
            X1CollectorConfig with_name(std::string name);

            /** copy of this config,
             *  but with @ref n_generation_ set to @p n_gen
             **/
            X1CollectorConfig with_n_gen(uint32_t n_gen);

            /** copy of this config,
             *  but with @ref n_survive_ set to @p n_survive
             **/
            X1CollectorConfig with_n_survive(uint32_t n_survive);

            /** copy of this config,
             *  but with @c arena_config_.size_ set to @p gen_z
             *
             *  TODO: rename to with_halfspace_size()
             **/
            X1CollectorConfig with_size(std::size_t gen_z);

            /** copy of this config,
             *  but with @c debug_flag_ set to @p x
             **/
            X1CollectorConfig with_debug_flag(bool x);

            /** copy of this config,
             *  but with @c sanitize_flag_ set to @p x
                **/
            X1CollectorConfig with_sanitize_flag(bool x);

            /** fetch configuration for gc object store **/
            GCObjectStoreConfig gco_store_config() const noexcept {
                return GCObjectStoreConfig(arena_config_,
                                           n_generation_,
                                           n_survive_threshold_,
                                           object_types_z_,
                                           debug_flag_);
            }

            /** fetch configuration for mutation log store **/
            MutationLogConfig mlog_config() const noexcept {
                bool mlog_enabled_flag = allow_incremental_gc_;

                return MutationLogConfig(n_generation_,
                                         mutation_log_z_,
                                         mlog_enabled_flag,
                                         debug_flag_);
            }

            Generation age2gen(object_age age) const noexcept {
                return this->gco_store_config().age2gen(age);
            }

            /** age threshold for promotion to generation @p g **/
            uint32_t promotion_threshold(Generation g) const noexcept {
                return this->gco_store_config().promotion_threshold(g);
            }

        public:
            // ----- Instance Variables -----

            /** optional name, for diagnostics **/
            std::string name_;

            /** Configuration for collector spaces.
             *  Will have (2 x G) of these,
             *  where G is @ref n_generation_.
             *  Not using name_ member.
             *
             *  REQUIRE:
             *  - arena_config_.store_header_flag_ must be true
             **/
            ArenaConfig arena_config_ = ArenaConfig().with_store_header_flag(true);

            /** storage for xgen pointer bookkeeping (aka remembered sets).
             *  Use 3x this value per generation
             **/
            std::size_t mutation_log_z_ = 1024;

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

            /** true to enable sanitize features:
             *  1. zero out from-space at end of GC cycle
             **/
            bool sanitize_flag_ = false;

            /** true to enable debug logging **/
            bool debug_flag_ = false;
        };


    } /*namespace mm*/
} /*namespace xo*/

/* end X1CollectorConfig.hpp */
