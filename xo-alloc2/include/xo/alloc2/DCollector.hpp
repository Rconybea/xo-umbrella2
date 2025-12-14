/** @file DCollector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "ArenaConfig.hpp"
#include "DArena.hpp"
#include <array>

namespace xo {
    namespace mm {
        template <typename T>
        using up = std::unique_ptr<T>;

        struct CollectorConfig {
            using size_type = std::size_t;

            /** Configuration for collector spaces.
             *  Will have at least {nursery,tenured} x {from,to} spaces.
             *  Not using name_ member.
             **/
            ArenaConfig arena_config_;

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

        /** State associated with a single DCollector generation
         **/
        struct Generation {
            Generation(uint8_t gen_id, up<DArena> from_space, up<Darena> to_space);
            ~Generation() = default;

            /** identity of this generation.  Generations are numbered from
             *  0 (youngest) to N (oldest), with N <= c_max_generation
             **/
            uint8_t gen_id_;
            /** from-space.  empty between collection episodes.
             *  During collection holds former to-space
             **/
            up<DArena> from_space_;
            /** to-space. New allocations occur here **/
            up<DArena> to_space_;
        };

        struct DCollector {
            std::uint32_t

            std::array<up<DArena>, c_max_generation> generations_[2];
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end DCollector.hpp */
