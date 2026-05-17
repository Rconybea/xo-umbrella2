/** @file GCObjectStore.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include "GCObjectStoreConfig.hpp"
#include "ObjectTypeSlot.hpp"
#include "object_age.hpp"
#include <xo/alloc2/role.hpp>
#include <array>

namespace xo {
    namespace mm {
        class X1VerifyStats;

        /** @brief container to hold gc-aware objects for X1 collector
         *
         *  Note: X1VerifyStats are in DX1Collector.
         *        They need to be there, since also interact with MutationLogStore.
         *        This is reason for DX1Collector to invoke .verify_aux()
         *        so it can supply X1VerifyStats location
         **/
        class GCObjectStore {
        public:
            using ObjectTypeTable = DArenaVector<ObjectTypeSlot>;
            /* TODO: AllocIterator pointing to free pointer instead of std::byte* */
            using GCMoveCheckpoint = std::array<std::byte *, c_max_generation>;
            using header_type = DArena::header_type;
            using value_type = DArena::value_type;
            using size_type = DArena::size_type;
            using typeseq = xo::reflect::typeseq;

        public:
            explicit GCObjectStore(const GCObjectStoreConfig & cfg, X1VerifyStats * p_verify_stats);

            const GCObjectStoreConfig & config() const noexcept { return config_; }

            const ObjectTypeTable * get_object_types() const noexcept { return &object_types_; }
            const DArena * get_space(Role r, Generation g) const noexcept { return space_[r][g]; }
            const DArena * from_space(Generation g) const noexcept { return this->get_space(Role::from_space(), g); }
            const DArena * to_space(Generation g) const noexcept { return this->get_space(Role::to_space(), g); }
            const DArena * new_space() const noexcept { return this->get_space(Role::to_space(), Generation{0}); }

            DArena * get_space(Role r, Generation g) noexcept { return space_[r][g]; }
            DArena * from_space(Generation g) noexcept { return this->get_space(Role::from_space(), g); }
            DArena * to_space(Generation g) noexcept { return this->get_space(Role::to_space(), g); }
            DArena * new_space() noexcept { return this->get_space(Role::to_space(), Generation{0}); }
            X1VerifyStats * verify_stats() noexcept { return p_verify_stats_; }

            /** true iff type with id @p tseq has known metadata
             *  (i.e. has appeared in preceding call to install_type
             *   for this collector)
             **/
            bool is_type_installed(typeseq tseq) const noexcept;

            /** lookup interface from type sequence
             *  (can use tseq = typeseq::id<T>() for type T)
             **/
            AGCObject * lookup_type(typeseq tseq) const noexcept;

            /** report allocated memory for role r, generation g
             **/
            size_type allocated(Generation g, Role r) const noexcept;

            /** report committed memory for role r, generation g
             **/
            size_type committed(Generation g, Role r) const noexcept;

            /** report reserved memory for role r, generation g
             **/
            size_type reserved(Generation g, Role r) const noexcept;

            /** generation to which pointer @p addr belongs, given Role @p r;
             *  sentinel if not found in this collector
             **/
            Generation generation_of(Role r, const void * addr) const noexcept;

            /** return details from last error (i.e. from g0 to-space) **/
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
            AllocInfo alloc_info(value_type mem) const noexcept;

            /** Call @p visitor for each memory pool owned by this store **/
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

            /** Report per-age-bucket information as an array of dictionaries.
             *  Scans to-space to count per-age statistics.
             *  Each dictionary has keys "n-live" and "bytes".
             *  Array index corresponds to object age.
             *
             *  @p mm        allocate stats from this allocator.
             *  @p error_mm  allocator for error reporting when out-of-memory.
             *  @p p_output  on exit @p *p_output contains stats array
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

            /** snap checkpoint containing allocator state
             *  use to detect forwarding activity after visiting objects
             **/
            GCMoveCheckpoint snap_move_checkpoint(Generation upto);

            /** verify consistency of this object store, on behalf of collector @p gc.
             *  Advancing counters in @p *p_verify_stats.
             *
             *  @p gc argument is load-bearing so we have collector interface
             *  to call AGCObject visitor method (forward_children()) on each
             *  object stored here.
             **/
            void verify_ok() noexcept;

            /** Register object type with this collector.
             *  Provides shallow copy and pointer forwarding for instances of this
             *  type.
             **/
            bool install_type(const AGCObject & meta) noexcept;

            /** For each generation g in [0 ,.., upto)
             *  swap arenas assigned to {to-space, from-space}.
             *  Invoked once at the beginning of each gc cycle.
             **/
            void swap_roles(Generation upto) noexcept;

            /** move subgraph at @p root to to-space on behalf of collector @p gc
             *  Special behavior relative to @ref _deep_move_interior :
             *  If @p root is not in gc-space, visit immediate children and move them in place (!).
             *
             *  @return new address for @p from_src
             *
             *  Require: runstate_.is_running()
             **/
            void * deep_move_root(const AGCObject * root_iface,
                                  void ** root_data,
                                  Generation upto);

            /** move interior subgraph at @p from_src to to-space.
             *  no-op if not in gc-space.
             *
             *  NOTE: load-bearing for MutationLogStore
             **/
            void * deep_move_interior(obj<AGCObjectVisitor> gc,
                                      void * from_src,
                                      Generation upto);

            /** allocate copy of @p src on behalf of a collection cycle.
             *  Entry point for DGCObjectStoreVisitor::alloc_copy()
             **/
            std::byte * alloc_copy(void * src) noexcept;

            /** Cleanup at the end of a gc cycle.
             *  Reset from-space
             *  (current from-space is former to-space,
             *  relabeled at the beginning of collector cycle)
             *  for generations in [0 ,.., upto)
             **/
            void cleanup_phase(Generation upto,
                               bool sanitize_flag);

            /** Revert to empty state **/
            void clear();

        private:

            /** configure @ref object_types_, using @p page_z **/
            void _init_object_types(std::size_t page_z);

            /** auxiliary init function **/
            void _init_space();

            /** true iff {@p alloc_hdr, @p object_data} should move for
             *  a collection of all generations strictly younger than @p upto.
             *
             *  Require: runstate_.is_running()
             **/
            bool _check_move_policy(header_type alloc_hdr,
                                    void * gco_data,
                                    Generation upto) const noexcept;

            /** Common driver for _deep_move_root(), _deep_move_interior().
             *  Move object subgraph @p from_src on behalf of @p gc collection cycle,
             *  covering generations in [0 ,.., upto).
             **/
            void * _deep_move_gc_owned(obj<AGCObjectVisitor> gc,
                                       void * from_src,
                                       Generation upto);

            /** traverse objects allocated after @p ckp, to make sure their children
             *  are forwarded. Repeat until traverse doesn't find any unforwarded children.
             *
             *  1. Breadth-first implementation, bad for memory locality
             *  2. Need @p gc for per-object-type forward_children api
             **/
            void _forward_children_until_fixpoint(obj<AGCObjectVisitor> gc,
                                                  Generation upto,
                                                  GCMoveCheckpoint gray_lo_v);

            /** during a gc cycle:
             *  evacuate object @p from_src, with gc-object interface @p iface.
             *  Shallow: does not traverse children
             **/
            void * _shallow_move(obj<AGCObjectVisitor> gc,
                                 AGCObject * iface,
                                 void * from_src);

            /** Evacuate object at @p *lhs_data to to-space, during collection phase
             *  acting on generations g in [0 ,.., upto).
             *  Need @p gc to pass to invoke AGCObject methods shallow_copy() and
             *  forward_children()
             *
             *  Replace original with forwarding pointer to new location
             **/
            void _forward_inplace_aux(obj<AGCObjectVisitor> gc,
                                      AGCObject * lhs_iface,
                                      void ** lhs_data,
                                      Generation upto);

            /** categorize fop {@p lhs_iface, @p lhs_data}
             *  based on location of @p lhs_data.
             *  Update @ref p_verify_stats_ based on the result:
             *  increment exactly one of {n_from_, n_to_, n_ext_}
             **/
            void _verify_aux(AGCObject * lhs_iface,
                             void * lhs_data);

            friend class DGCObjectStoreVisitor;

        private:
            /** configuration for gc-aware object store **/
            GCObjectStoreConfig config_;

            /** gc-aware object types **/
            ObjectTypeTable object_types_;

            /** arena objects for collector managed memory
             *  1:1 with roles, but polarity reverses for each collection
             **/
            std::array<DArena, c_max_generation> space_storage_[c_n_role];

            /** arena pointers. The roles of space_storage_[0][g] and space_storage_[1][g]
             *  are reversed each time generation g gets collected.
             **/
            std::array<DArena*, c_max_generation> space_[c_n_role];

            /** dedicated counters. updated by .verify_aux() **/
            X1VerifyStats * p_verify_stats_ = nullptr;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end GCObjectStore.hpp */
