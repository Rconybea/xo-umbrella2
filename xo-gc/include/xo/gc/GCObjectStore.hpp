/** @file GCObjectStore.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include "GCObjectStoreConfig.hpp"
#include "generation.hpp"
#include "object_age.hpp"
#include <xo/alloc2/role.hpp>
//#include <xo/arena/ArenaConfig.hpp>
#include <array>

namespace xo {
    namespace mm {

        /** @brief container to hold gc-aware objects for X1 collector
         **/
        class GCObjectStore {
        public:
            using header_type = DArena::header_type;
            using value_type = DArena::value_type;
            using size_type = DArena::size_type;

        public:
            explicit GCObjectStore(const GCObjectStoreConfig & cfg);

            const DArena * get_space(role r, Generation g) const noexcept { return space_[r][g]; }
            DArena * get_space(role r, Generation g) noexcept { return space_[r][g]; }
            DArena * from_space(Generation g) noexcept { return get_space(role::from_space(), g); }
            DArena * to_space(Generation g) noexcept { return get_space(role::to_space(), g); }
            DArena * new_space() noexcept { return to_space(Generation{0}); }

            /** generation to which pointer @p addr belongs, given role @p r;
             *  sentinel if not found in this collector
             **/
            Generation generation_of(role r, const void * addr) const noexcept;

            /** get allocation size from header **/
            std::size_t header2size(header_type hdr) const noexcept;
            /** get generation counter from alloc header **/
            object_age header2age(header_type hdr) const noexcept;
            /** get tseq from alloc header **/
            uint32_t header2tseq(header_type hdr) const noexcept;

            /** true iff original alloc has been replaced by a forwarding pointer **/
            bool is_forwarding_header(header_type hdr) const noexcept;

            /** Retreive bookkeeping info for allocation at @p mem. **/
            AllocInfo alloc_info(value_type mem) const noexcept;

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
            /** configuration for gc-aware object store **/
            GCObjectStoreConfig config_;

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
