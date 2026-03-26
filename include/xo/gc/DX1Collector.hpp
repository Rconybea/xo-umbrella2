/** @file DX1Collector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "X1CollectorConfig.hpp"
#include "GCObject.hpp"
#include "MutationLogEntry.hpp"
#include "generation.hpp"
#include "object_age.hpp"
#include "role.hpp"
#include <xo/alloc2/Allocator.hpp>
#include <xo/arena/DArena.hpp>
#include <xo/arena/DArenaVector.hpp>
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

        // ----- GCRunState -----

        /** @class GCRunState
         *  @brief encapsulate state needed while GC is running
         **/
        struct GCRunState {
            enum class Mode {
                /** gc not running. X1 available for normal allocation **/
                idle,
                /** gc in progress. X1 not available for normal allocation **/
                gc,
                /** verify in progress. @ref verify_ok call is on the stack **/
                verify
            };

            GCRunState() : gc_upto_{0} {}
            GCRunState(Mode mode, generation gc_upto);

            static GCRunState idle();
            static GCRunState verify();
            static GCRunState gc_upto(generation g);

            generation gc_upto() const { return gc_upto_; }

            bool is_running() const { return mode_ == Mode::idle; }
            bool is_verify() const { return mode_ == Mode::verify; }

        private:
            /** current collector mode **/
            Mode mode_;
            /** running gc collecting all generations gi < gc_upto **/
            generation gc_upto_;
        };

        struct DX1CollectorIterator;

        /** @brief GC root struct
         *
         *  A root is traversed much like other gc-owned value:
         *  a. if root is in GC from-space, move it to to-space.
         *  b. if root is in GC to-space, skip.
         *     e.g. root belongs to generation not subject to collection this cycle.
         *  c. if root it not allocated by GC, still do in-place forward on its
         *     children.  This is load-bearing for ParserStateMachine, for example.
         *     Allows non-GC object to refer to a dynamic set of gc-owned objects
         **/
        struct GCRoot {
        public:
            GCRoot() = default;
            explicit GCRoot(obj<AGCObject> * x) : root_{x} {}

            obj<AGCObject> * root() { return root_; }

        private:
            obj<AGCObject> * root_ = nullptr;
        };

        /** @brief info collected during a @ref DX1Collector::verify_ok call
         *
         **/
        struct VerifyStats {
            void clear() { *this = VerifyStats(); }

            std::uint32_t n_ext_  = 0;
            std::uint32_t n_from_ = 0;
            std::uint32_t n_to_   = 0;
        };

        // ----- DX1Collector -----

        /** @brief garbage collector 'X1'
         **/
        struct DX1Collector {
        public:
            using RootSet = DArenaVector<GCRoot>;
            using MutationLog = DArenaVector<MutationLogEntry>;
            using typeseq = xo::facet::typeseq;
            using size_type = DArena::size_type;
            using value_type = DArena::value_type;
            using header_type = DArena::header_type;

            /** hard max typeseq for collector-registered types **/
            static constexpr size_t c_max_typeseq = 4096;

        public:
            /** Create X1 collector instance. **/
            explicit DX1Collector(const X1CollectorConfig & cfg);

            /** faceted object pointer to this instance */
            template <typename AFacet = AAllocator>
            obj<AFacet,DX1Collector> ref() { return obj<AFacet,DX1Collector>(this); }

            // ----- access methods -----

            std::string_view name() const noexcept { return config_.name_; }
            GCRunState runstate() const noexcept { return runstate_; }
            const DArena * get_object_types() const noexcept { return &object_types_; }
            const RootSet * get_root_set() const noexcept { return &root_set_; }
            const DArena * get_space(role r, generation g) const noexcept { return space_[r][g]; }
            DArena * get_space(role r, generation g) noexcept { return space_[r][g]; }
            DArena * from_space(generation g) noexcept { return get_space(role::from_space(), g); }
            DArena * to_space(generation g) noexcept { return get_space(role::to_space(), g); }
            DArena * new_space() noexcept { return to_space(generation{0}); }

            // ----- basic statistics -----

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

            // ----- queries -----

            /** introspection for memory use.
             *  Call @p visitor(info) for each pool owned by this allocator
             **/
            void visit_pools(const MemorySizeVisitor & visitor) const;

            /** true iff address @p addr allocated from this collector
             *  in role @p r (according to current GC state)
             **/
            bool contains(role r, const void * addr) const noexcept;

            /** true iff address @p addr allocated from this collector and currently live
             *  in role @p r (according to current GC state)
             **/
            bool contains_allocated(role r, const void * addr) const noexcept;

            /** generation to which pointer @p addr belongs, given role @p r;
             *  sentinel if not found in this collector
             **/
            generation generation_of(role r, const void * addr) const noexcept;

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

            /** verify that GC state appears consistent **/
            bool verify_ok() noexcept;

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

            // ------ gc root management -----

            /** add GC root at @p *p_root **/
            void add_gc_root_poly(obj<AGCObject> * p_root) noexcept;

            /** remove GC root at @p *p_root **/
            void remove_gc_root_poly(obj<AGCObject> * p_root) noexcept;

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

            // ----- mutation -----

            /** Modify a gc-owned pointer @p *p_lhs, within allocation @p parent,
             *  to point to @p rhs.
             *
             *  Motivation: need special handling for cross-generational pointers with
             *  incremental gc.
             *
             *  Require:
             *  - if parent is owned by this collector, it has it's own allocation
             *    (alloc header immediately precedes object address @c parent.data_)
             *  - address @p p_lhs falls within extent of allocation for @c parent.data_
             **/
            void assign_member(void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> rhs);

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
            /** aux init function: initialize @ref object_types_ arena **/
            void _init_object_types(const X1CollectorConfig & cfg, std::size_t page_z);
            /** aux init function: initialize @ref roots_ arena **/
            void _init_gc_roots(const X1CollectorConfig & cfg, std::size_t page_z);
            /** aux init function: initialize @ref mlog_storage_[][] arenas **/
            void _init_mlogs(const X1CollectorConfig & cfg, std::size_t page_z);
            /** aux init function: create mutation log **/
            MutationLog _make_mlog(uint32_t igen, char tag_char, size_t mlog_z, std::size_t page_z);
            /** aux init function: initialize @ref space_storage_[][] arenas **/
            void _init_space(const X1CollectorConfig & cfg);

            /** swap from- and to- roles for all generations < @p upto **/
            void swap_roles(generation upto) noexcept;
            /** copy roots + everything reachable from them, to to-space **/
            void copy_roots(generation upto) noexcept;

            /** cleanup after gc **/
            void cleanup_phase(generation upto);

            /** move root subgraph at @p from_src to to-space.
             *  If not in gc-space, visit immediate children and move them.
             *  Require: runstate_.is_running()
             **/
            void * _deep_move_root(obj<AGCObject> from_src, generation upto);
            /** move interior subgraph at @p from_src to to-space.
             *  no-op if not in gc-space.
             **/
            void * _deep_move_interior(void * from_src, generation upto);
            /** Common driver for _deep_move_root(), _deep_move_interior() **/
            void * _deep_move_gc_owned(void * from_src, generation upto);
            /** Evacuate object at @p *lhs_data to to-space.
             *  Replace original with forwarding pointer to new location
             **/
            void _forward_inplace_aux(AGCObject * lhs_iface, void ** lhs_data);
            /** Verify that pointer {@p iface, @p data} is valid:
             *  destination either in to-space, or somewhere outside this collector
             **/
            void _verify_aux(AGCObject * iface, void * data);

        public:
            /** garbage collector configuration **/
            X1CollectorConfig config_;

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

            /** using arena to get extensible list of root objects.
             *  For each root store one address (type obj<AGCObject>*)
             *
             *  An Object x that supports AGCObject, but doesn't live in gc-space,
             *  will get special treatment if it appears in root_set_:
             *  collector will traverse x to forward pointers to gc-owned
             *  targets. In other contexts collector doesn't look inside
             *  non-gc-owned objects
             *
             *  editor bait: root_v
             **/
            RootSet root_set_;

            /** Cross-generational mutations tracked in MutationLogs.
             *  We need three logs per generation:
             *  A. one to observe and remember mutations in to-space
             *     during normal operation (between GC cycles)
             *  B. during GC: 2nd mlog to hold entries from from-mlog
             *     that will still be needed post-GC (because ptr direction
             *     from higher gen to lower gen after cycle).
             *  C. during GC: 3rd mlog to triage entries for which
             *     liveness of pointer source isn't yet established.
             *
             * NOTE: indexed on generation of pointer *destination*
             **/
            std::array<MutationLog, c_max_generation - 1> mlog_storage_[c_n_role + 1];

            /** mlog pointers.  The roles of mlog_storage_[*][g] get permuted
             *  as each collection cycle proceeds
             **/
            std::array<MutationLog *, c_max_generation - 1> mlog_[c_n_role + 1];

            /** collector-managed memory here.
             *  - space_[1] is from-space
             *  - space_[0] is to-space
             *  coordinates with role in gc/role.hpp, see also.
             **/

            /** arena objects for collector managed memory
             *  1:1 with roles, but polarity reverses for each collection
             **/
            std::array<DArena, c_max_generation> space_storage_[c_n_role];

            /** arena pointers. The roles of space_storage_[0][g] and space_storage_[1][g]
             *  are reversed each time generation g gets collected.
             **/
            std::array<DArena*, c_max_generation> space_[c_n_role];

            /** counters collected during @ref verify_ok call **/
            VerifyStats verify_stats_;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1Collector.hpp */
