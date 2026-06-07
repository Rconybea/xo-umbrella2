/** @file DX1Collector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "X1CollectorConfig.hpp"
#include "GCObjectStore.hpp"
#include "MutationLogStore.hpp"
#include "X1VerifyStats.hpp"
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

            GCRunState() : mode_{Mode::idle}, gc_upto_{0} {}
            GCRunState(Mode mode, Generation gc_upto);

            static GCRunState idle();
            static GCRunState verify();
            static GCRunState gc_upto(Generation g);

            Generation gc_upto() const { return gc_upto_; }

            bool is_idle() const { return mode_ == Mode::idle; }
            bool is_running() const { return mode_ == Mode::gc; }
            bool is_verify() const { return mode_ == Mode::verify; }

        private:
            /** current collector mode **/
            Mode mode_ = Mode::idle;
            /** running gc collecting all generations gi < gc_upto **/
            Generation gc_upto_;
        };

        /** @class GcStatistics
         **/
        struct GCStatistics {
        public:
            GCStatistics() = default;
            //explicit GCStatistics(uint32_t n_gc) : n_gc_{n_gc} {};

            uint32_t n_gc() const noexcept { return n_gc_; }

            void include_gc() {
                ++n_gc_;
            }

        private:
            /** count #gc **/
            uint32_t n_gc_ = 0;
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

        // ----- DX1Collector -----

        /** @brief garbage collector 'X1'
         **/
        class DX1Collector {
        public:
            using RootSet = DArenaVector<GCRoot>;
            using ObjectTypeTable = DArenaVector<ObjectTypeSlot>;
            /* TODO: AllocIterator pointing to free pointer instead of std::byte* */
            using GCMoveCheckpoint = std::array<std::byte *, c_max_generation>;
            using MutationLog = DArenaVector<MutationLogEntry>;
            using typeseq = xo::facet::typeseq;
            using size_type = GCObjectStore::size_type;
            using value_type = GCObjectStore::value_type;
            using header_type = GCObjectStore::header_type;

            /** hard max typeseq for collector-registered types **/
            static constexpr size_t c_max_typeseq = 4096;

        public:
            /** Create X1 collector instance. **/
            explicit DX1Collector(const X1CollectorConfig & cfg);

            /** faceted object pointer to this instance */
            template <typename AFacet = AAllocator>
            obj<AFacet,DX1Collector> ref() { return obj<AFacet,DX1Collector>(this); }

            // ----- access methods -----

            const X1CollectorConfig & config() const noexcept { return config_; }

            const GCObjectStore & gco_store() const noexcept { return gco_store_; }
            GCObjectStore & gco_store() noexcept { return gco_store_; }

            std::string_view name() const noexcept { return config_.name_; }
            GCRunState runstate() const noexcept { return runstate_; }
            const GCStatistics & gc_stats() const noexcept { return gc_stats_; }

            const ObjectTypeTable * get_object_types() const noexcept { return gco_store_.get_object_types(); }
            const RootSet * get_root_set() const noexcept { return &root_set_; }
            const DArena * get_space(Role r, Generation g) const noexcept { return gco_store_.get_space(r, g); }
            DArena * get_space(Role r, Generation g) noexcept { return gco_store_.get_space(r, g); }
            DArena * from_space(Generation g) noexcept { return this->get_space(Role::from_space(), g); }
            DArena * to_space(Generation g) noexcept { return this->get_space(Role::to_space(), g); }
            DArena * new_space() noexcept { return this->to_space(Generation{0}); }

            // ----- basic statistics -----

            /** total reserved memory in bytes, across all {Role, generation} **/
            size_type reserved() const noexcept;
            /** total size in bytes (same as committed_total()) **/
            size_type size_total() const noexcept;
            /** total committed memory in bytes, across all {Role, generation} **/
            size_type committed() const noexcept;
            /** total available memory in bytes, across all {Role, generation} **/
            size_type available() const noexcept;
            /** total allocated memory in bytes, across all {Role, generation} **/
            size_type allocated() const noexcept;

            /** total number of mutation log entries **/
            size_type mutation_log_entries() const noexcept;

            /** memory allocated for generation @p g in Role @p r **/
            size_type allocated(Generation g, Role r) const noexcept;
            /** memory committed for generation @p g in Role @p r **/
            size_type committed(Generation g, Role r) const noexcept;
            /** memory (virtual addresses) reserved for generation @p g in Role @p r **/
            size_type reserved(Generation g, Role r) const noexcept;

            /** very similar to generation_of(), but satisfies ACollector api **/
            std::int32_t locate_address(const void * addr) const noexcept;

            // ----- full statistics -----

            /** Report gc statistics as a dictionary.
             *  Providing for the same of making GC statistics visible to schematika programs
             *
             *  @p mm        allocate stats dictionary from this allocator.
             *               May be the same as this collector.
             *  @p error_mm  Allocator for last-report error reporting when out-of-memory.
             *  @p p_output  on exit @p *p_output contains stats dictionary
             **/
            bool report_statistics(obj<AAllocator> mm,
                                   obj<AAllocator> error_mm,
                                   obj<AGCObject> * p_output) const noexcept;

            /** Report per-object-type information as a dictionary.
             *  Scans to-space to count per-object-type information
             *
             *  @p mm        allocate stats dictionary from this allocator.
             *               May be the same as this collector.
             *  @p error_mm  Allocator for last-report error reporting when out-of-memory.
             *  @p p_output  on exit @p *p_output contains stats dictionary
             **/
            bool report_object_types(obj<AAllocator> mm,
                                     obj<AAllocator> error_mm,
                                     obj<AGCObject> * p_output) const noexcept;

            /** Report per-age-bucket information as an array of dictionaries.
             *  Scans to-space to count per-age statistics.
             *  Each dictionary has keys "n-live" and "bytes".
             *  Array index corresponds to object age.
             *
             *  @p mm        allocate stats from this allocator.
             *  @p error_mm  allocator for error reporting when out-of-memory.
             *  @p p_output  on exit @p *p_output contains stats array
             **/
            bool report_object_ages(obj<AAllocator> mm,
                                    obj<AAllocator> error_mm,
                                    obj<AGCObject> * p_output) const noexcept;

            // ----- queries -----

            /** introspection for memory use.
             *  Call @p visitor(info) for each pool owned by this allocator
             **/
            void visit_pools(const MemorySizeVisitor & visitor) const;

            /** true iff address @p addr allocated from this collector
             *  in Role @p r (according to current GC state)
             **/
            bool contains(Role r, const void * addr) const noexcept;

            /** true iff address @p addr allocated from this collector and currently live
             *  in Role @p r (according to current GC state)
             *
             *  (i.e. in [lo,free) for an arena)
             **/
            bool contains_allocated(Role r, const void * addr) const noexcept;

            /** generation to which pointer @p addr belongs, given Role @p r;
             *  sentinel if not found in this collector
             **/
            Generation generation_of(Role r, const void * addr) const noexcept;

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

            /** Retreive bookkeeping info for allocation at @p mem. **/
            AllocInfo alloc_info(void * mem) const noexcept;

            /** true iff type with id @p tseq has known metadata
             *  (i.e. has appeared in preceding call to install_type
             *   for this collector)
             **/
            bool is_type_installed(typeseq tseq) const noexcept;

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
            void request_gc(Generation upto) noexcept;

            /** Execute gc immediately, for all generations < @p upto **/
            void execute_gc(Generation upto) noexcept;

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

            /** DEPRECATED. Prefer .barrier_assign_aux().
             *
             *  Modify a gc-owned pointer @p *p_lhs, within allocation @p parent,
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

            /** discard all allocated memory **/
            void clear() noexcept;

            /** perform fop assignment of (rhs_iface,rhs_data)
             *  to (lhs_iface,lhs_data) within allocation @parent
             *  + create mlog entry if necessary.
             **/
            void barrier_assign_aux(void * parent,
                                    AGCObject * lhs_iface, void ** lhs_data,
                                    AGCObject * rhs_iface, void * rhs_data);

        private:
            /** aux init function: initialize @ref roots_ arena **/
            void _init_gc_roots(const X1CollectorConfig & cfg, std::size_t page_z);
            /** aux init function: initialize @ref mlog_storage_[][] arenas **/
            void _init_mlogs(std::size_t page_z);
            /** aux init function: initialize @ref space_storage_[][] arenas **/
            void _init_space(const X1CollectorConfig & cfg);

            /** swap from- and to- Roles for all generations < @p upto **/
            void _swap_roles(Generation upto) noexcept;
            /** copy roots + everything reachable from them, to to-space **/
            void _copy_roots(Generation upto) noexcept;

            /** cleanup after gc **/
            void _cleanup_phase(Generation upto);

#ifdef OBSOLETE
            /** Verify that pointer {@p iface, @p data} is valid:
             *  destination either in to-space, or somewhere outside this collector
             **/
            void _verify_aux(AGCObject * iface, void * data);
#endif

        public:
            /** garbage collector configuration **/
            X1CollectorConfig config_;

            /** current gc state **/
            GCRunState runstate_;

            /** gc disabled whenever gc_blocked_ > 0 **/
            uint32_t gc_blocked_ = 0;

            /** if > 0: need gc for all generations < gc_pending_upto_ **/
            Generation gc_pending_upto_;

            /**
             *  An Object x that supports AGCObject, but doesn't live in gc-space,
             *  will get special treatment if it appears in root_set_:
             *  collector will traverse x to forward pointers to gc-owned
             *  targets. In other contexts collector doesn't look inside
             *  non-gc-owned objects
             *
             *  editor bait: root_v
             **/
            RootSet root_set_;

            /** Collector-managed memory.
             **/
            GCObjectStore gco_store_;

            /** "remembered sets": track pointers P->C that require special handling
             *  during a gc cycle where either:
             *  1. xgen pointers g(P) > g(C):
             *     P in a more senior generation than C
             *  2. xage pointers g(P) = g(C), age(P) > age(C):
             *     {P,C} in same generation, but in fuutre suriving P would
             *     get promoted before C.
             **/
            MutationLogStore mlog_store_;

            /** counters collected across GC phases **/
            GCStatistics gc_stats_;

            /** counters collected during @ref verify_ok call **/
            X1VerifyStats verify_stats_;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1Collector.hpp */
