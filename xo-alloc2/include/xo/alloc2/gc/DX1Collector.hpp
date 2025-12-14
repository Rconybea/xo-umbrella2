/** @file DX1Collector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "ArenaConfig.hpp"
#include "DArena.hpp"
#include "gc/generation.hpp"
#include "gc/role.hpp"
#include <memory>
#include <array>

namespace xo {
    namespace mm {
        template <typename T>
        using up = std::unique_ptr<T>;

#ifdef NOT_YET
        /** State associated with a single DX1Collector generation
         **/
        struct Generation {
            Generation(uint8_t gen_id, up<DArena> from_space, up<DArena> to_space);
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
#endif

        struct CollectorConfig {
            using size_type = std::size_t;

            /** Configuration for collector spaces.
             *  Will have at least {nursery,tenured} x {from,to} spaces.
             *  Not using name_ member.
             *
             *  REQUIRE:
             *  - arena_config_.store_header_flag_ must be true
             *  - arena_config_.header_size_mask_ must be 0x0000ffff
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

        struct DX1Collector {
            explicit DX1Collector(const CollectorConfig & cfg);

            DArena * get_space(role r, generation g) { return space_[r][g]; }
            DArena * from_space(generation g) { return get_space(role::from_space(), g); }
            DArena * to_space(generation g) { return get_space(role::to_space(), g); }

            /** reverse to-space and from-space roles for generation g **/
            void reverse_roles(generation g);

            /** garbage collector configuration **/
            CollectorConfig config_;

            /** collector-managed memory here.
             *  - space_[1] is from-space
             *  - space_[0] is to-space
             *  coordinates with role ingc/role.hpp, see also.
             **/

            /** arena objects for collector managed memory
             *  1:1 with roles, but polarity reverses for each collection
             **/
            std::array<DArena, c_max_generation> space_storage_[c_n_role];

            /** arena pointers. The roles of space_storage_[0][g] and space_storage_[1][g]
             *  are reversed each time generation g gets collected.
             **/
            std::array<DArena*, c_max_generation> space_[c_n_role];
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1Collector.hpp */
