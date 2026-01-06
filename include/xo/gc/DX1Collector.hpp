/** @file DX1Collector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "GCObject.hpp"
#include "generation.hpp"
#include "object_age.hpp"
#include "role.hpp"
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/arena/DArena.hpp>
#include <xo/arena/ArenaConfig.hpp>
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

        // ----- GCRunState -----

        /** @class GCRunState
         *  @brief encapsulate state needed while GC is running
         **/
        struct GCRunState {
            GCRunState() : gc_upto_{0} {}
            explicit GCRunState(generation gc_upto);

            static GCRunState gc_not_running();
            static GCRunState gc_upto(generation g);

            generation gc_upto() const { return gc_upto_; }

            bool is_running() const { return gc_upto_ > 0; }

        private:
            /** running gc collecting all generations gi < gc_upto **/
            generation gc_upto_;
        };

        struct DX1CollectorIterator;

        // ----- DX1Collector -----

        struct DX1Collector {
            using typeseq = xo::facet::typeseq;
            using size_type = DArena::size_type;
            using value_type = DArena::value_type;
            using header_type = DArena::header_type;

            /** hard max typeseq for collector-registered types **/
            static constexpr size_t c_max_typeseq = 4096;

            /** Create X1 collector instance. **/
            explicit DX1Collector(const CollectorConfig & cfg);

            std::string_view name() const { return config_.name_; }

            const DArena * get_object_types() const noexcept { return &object_types_; }
            const DArena * get_roots() const noexcept { return &roots_; }
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

            /** true iff type with id @p tseq has known metadata
             *  (i.e. has appeared in preceding call to install_type
             *   for this collector)
             **/
            bool is_type_installed(typeseq tseq) const noexcept;

            /** Retreive bookkeeping info for allocation at @p mem. **/
            AllocInfo alloc_info(value_type mem) const noexcept;

            // ----- app memory model -----

            /** lookup interface from type sequence
             *  (can use tseq = typeseq::id<T>() for type T)
             **/
            const AGCObject * lookup_type(typeseq tseq) const noexcept;

            /** Register object type with this collector.
             *  Provides shallow copy and pointer forwarding for instances of this
             *  type.
             **/
            bool install_type(const AGCObject & meta) noexcept;

            /** add GC root at @p root_addr, with type @p typeseq **/
            void add_gc_root_poly(obj<AGCObject> * p_root) noexcept;

            // ----- collection -----

            /** Request immediate collection.
             *  1. if collection is enabled, immediately collect all generations
             *     up to (but not including) g
             *  2. may nevertheless escalate to older generations,
             *     depending on collector state.
             *  3. if collection is currently disabled,
             *     collection will trigger the next time gc is enabled.
             **/
            void request_gc(generation upto) noexcept;

            /** Execute gc immediately, for all generations < @p upto **/
            void execute_gc(generation upto) noexcept;

            /** Evacuate object at @p *lhs_data to to-space.
             *  Replace original with forwarding pointer to new location
             **/
            void forward_inplace(AGCObject * lhs_iface, void ** lhs_data);

            /** evacuate object with type @p iface at address @p from_src
             *  to to-space. Return new to-space location.
             **/
            void * shallow_move(const AGCObject * iface, void * from_src);

            /** true iff {alloc_hdr, object_data} should move for
             *  currently-running collection.
             *
             *  Require: runstate_.is_running()
             **/
            bool check_move_policy(header_type alloc_hdr, void * object_data) const noexcept;

            // ----- allocation -----

            /** simple allocation. allocate @p z bytes of memory
             *  for an object of type @p t.
             *  New allocs always in gen0 to-space
             **/
            value_type alloc(typeseq t, size_type z) noexcept;
            /** compound allocation. Allocate @p z bytes of memory
             *  for an object of type @p t.
             *  To be followed immediately by:
             *  1. zero or more calls to sub_alloc(zi, complete=false), then
             *  2. exactly one call to sub_alloc(zi, complete=true)
             *  all the allocs in a compound allocation share the same
             *  allocation header. End state is equivalent to a single
             *  allocation with size z + sum(zi).
             *  New allocs always in gen0 to-space
             **/
            value_type super_alloc(typeseq t, size_type z) noexcept;
            /** sub-allocation with preceding compound allocation.
             *  New allocs always in gen0 to-space
             **/
            value_type sub_alloc(size_type z, bool complete) noexcept;
            /** Allocate copy of source object at @p src.
             *  Source must be owned by this collector instance.
             *  Copy will have incremented age.
             **/
            value_type alloc_copy(value_type src) noexcept;
            /** expand gen0 committed size to at least @p z.
             **/
            bool expand(size_type z) noexcept;

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

        private:
            /** swap from- and to- roles for all generations < @p upto **/
            void swap_roles(generation upto) noexcept;
            /** copy roots + everything reachable from them, to to-space **/
            void copy_roots(generation upto) noexcept;

            /** move subgraph at @p from_src to to-space.
             *
             *  Require: runstate_.is_running()
             **/
            void * deep_move(void * from_src, generation upto);

        public:
            /** garbage collector configuration **/
            CollectorConfig config_;

            /** current gc state **/
            GCRunState runstate_;

            /** (ab)using arena to get an extensible array of object types.
             *  For each type need to store one (8-byte) IGCObject_Any instance,
             **/
            DArena object_types_;

            /** gc disabled whenever gc_blocked_ > 0 **/
            uint32_t gc_blocked_ = 0;
            /** if > 0: need gc for all generations < gc_pending_upto_ **/
            generation gc_pending_upto_;

            /** (ab)using arena to get extensible list of root objects.
             *  For each root store one address (type obj<AGCObject>*)
             **/
            DArena roots_;

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
