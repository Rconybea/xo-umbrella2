/** @file GCObjectStore.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include "generation.hpp"
#include <xo/alloc2/role.hpp>
#include <xo/arena/DArena.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <array>

namespace xo {
    namespace mm {

        /** @brief container to hold gc-aware objects for X1 collector
         **/
        class GCObjectStore {
        public:
            GCObjectStore(const ArenaConfig & arena_cfg, uint32_t ngen, bool debug_flag);

            const DArena * get_space(role r, Generation g) const noexcept { return space_[r][g]; }
            DArena * get_space(role r, Generation g) noexcept { return space_[r][g]; }
            DArena * from_space(Generation g) noexcept { return get_space(role::from_space(), g); }
            DArena * to_space(Generation g) noexcept { return get_space(role::to_space(), g); }
            DArena * new_space() noexcept { return to_space(Generation{0}); }

            /** Call @p visitor for each memory pool owned by this store **/
            void visit_pools(const MemorySizeVisitor & visitor) const;

            /** For each generation g in [0 ,.., upto)
             *  swap arenas assigned to {to-space, from-space}.
             *  Invoked once at the beginning of each gc cycle.
             **/
            void swap_roles(Generation upto) noexcept;

            /** Cleanup at the end of a gc cycle.
             *  Reset from-space
             *  (current from-space is former to-space,
             *  relabeled at the beginning of collector cycle)
             *  for generations in [0 ,.., upto)
             **/
            void cleanup_phase(Generation upto,
                               bool sanitize_flag);

        private:
            /** auxiliary init function **/
            void _init_space();

        private:
            /** Configuration for collector spaces.
             *  Will have (2 x G) of these,
             *  where G is @ref n_generation_.
             *  Not using name_ member.
             *
             *  REQUIRE:
             *  - arena_config_.store_header_flag_ must be true
             **/
            ArenaConfig arena_config_;
            /** number of generations in use.  Same as @ref X1CollectorConfig::n_generation_ **/
            uint32_t n_generation_ = 0;
            /** true to enable debug logging **/
            bool debug_flag_ = false;

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

/* end GCObjectStore.hpp */
