/** @file DX1Collector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "arena/ArenaConfig.hpp"
#include "arena/DArena.hpp"
#include "gc/generation.hpp"
#include "gc/object_age.hpp"
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

#ifdef OBSOLETE // get from arena_config_.header_
            /*
             * alloc header
             *  TTTTTTTTTTTTGGGGGZZZZZZZZZZZZ
             *  <   tseq   ><gen><   size   >
             *
             * masking
             *
             *  ..432107654321076543210 bit
             *
             *                 >       <  .gen_bits
             *  0..............01111111   gen_mask_unshifted
             *  0..011111110..........0   gen_mask_shifted
             *             >           <  gen_shift
             */
            //constexpr std::uint64_t gen_mult() const;
            constexpr std::uint64_t gen_shift() const;
            constexpr std::uint64_t gen_mask_unshifted() const;
            constexpr std::uint64_t gen_mask_shifted() const;

            //constexpr std::uint64_t tseq_mult() const;
            constexpr std::uint64_t tseq_shift() const;
            constexpr std::uint64_t tseq_mask_unshifted() const;
            constexpr std::uint64_t tseq_mask_shifted() const;
#endif

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

        // ----- GCRunState -----

        /** @class GCRunState
         *  @brief encapsulate state needed while GC is running
         **/
        struct GCRunState {
            GCRunState() : gc_upto_{0} {}
            explicit GCRunState(generation gc_upto);

            static GCRunState gc_not_running();
            static GCRunState gc_upto(generation g);

            bool is_running() const { return gc_upto_ > 0; }

            generation gc_upto() const { return gc_upto_; }

        private:
            /** running gc collecting all generations gi < gc_upto **/
            generation gc_upto_;
        };

        struct DX1CollectorIterator;

        // ----- DX1Collector -----

        struct DX1Collector {
            using size_type = DArena::size_type;
            using value_type = DArena::value_type;
            using header_type = DArena::header_type;

            explicit DX1Collector(const CollectorConfig & cfg);

            const DArena * get_space(role r, generation g) const noexcept { return space_[r][g]; }
            DArena * get_space(role r, generation g) noexcept { return space_[r][g]; }
            DArena * from_space(generation g) noexcept { return get_space(role::from_space(), g); }
            DArena * to_space(generation g) noexcept { return get_space(role::to_space(), g); }
            DArena * new_space() noexcept { return to_space(generation{0}); }

            /** total reserved memory in bytes, across all {role, generation} **/
            size_type reserved_total() const noexcept;
            /** total size in bytes (same as committed_total()) **/
            size_type size_total() const noexcept;
            /** total committed memory in bytes, across all {role, generation} **/
            size_type committed_total() const noexcept;
            /** total available memory in bytes, across all {role, generation} **/
            size_type available_total() const noexcept;
            /** total allocated memory in bytes, across all {role, generation} **/
            size_type allocated_total() const noexcept;

            /** true iff address @p addr allocated from this collector
             *  in role @p r (according to current GC state)
             **/
            bool contains(role r, const void * addr) const noexcept;

            /** return details from last error (will be in gen0 to-space) **/
            AllocError last_error() const noexcept;

            /** get allocation size from header **/
            std::size_t header2size(header_type hdr) const noexcept;
            /** get generation counter from alloc header **/
            object_age header2age(header_type hdr) const noexcept;
            /** get tseq from alloc header **/
            uint32_t header2tseq(header_type hdr) const noexcept;

            /** true iff original alloc has been replaced by a forwarding pointer **/
            bool is_forwarding_header(header_type hdr) const noexcept;

            // ----- allocation -----

            /** simple allocation. new allocs always in gen0 to-space **/
            value_type alloc(size_type z) noexcept;
            /** compound allocation. To be followed immediately by:
             *  1. zero or more calls to sub_alloc(zi, complete=false), then
             *  2. exactly one call to sub_alloc(zi, complete=true)
             *  all the allocs in a compound allocation share the same
             *  allocation header. End state is equivalent to a single
             *  allocation with size z + sum(zi).
             *  New allocs always in gen0 to-space
             **/
            value_type super_alloc(size_type z) noexcept;
            /** sub-allocation with preceding compound allocation.
             *  New allocs always in gen0 to-space
             **/
            value_type sub_alloc(size_type z, bool complete) noexcept;
            /** expand gen0 committed size to at least @p z.
             **/
            bool expand(size_type z) noexcept;
            /** Retreive bookkeeping info for allocation at @p mem. **/
            AllocInfo alloc_info(value_type mem) noexcept;

            // ----- iteration -----

            /** alloc iterator at begin position **/
            DX1CollectorIterator begin() const noexcept;
            /** alloc iterator at end position
             *  (valid, but cannot be dereferenced)
             **/
            DX1CollectorIterator end() const noexcept;

            // ----- book-keeping -----

            /** reverse to-space and from-space roles for generation g **/
            void reverse_roles(generation g) noexcept;

            /** discard all allocated memory **/
            void clear() noexcept;

        public:
            /** garbage collector configuration **/
            CollectorConfig config_;

            /** current gc state **/
            GCRunState runstate_;

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
