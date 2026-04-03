/** @file GCObjectStore.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include "GCObjectStoreConfig.hpp"
#include "ObjectTypeSlot.hpp"
//#include "Generation.hpp"
#include "object_age.hpp"
#include <xo/alloc2/role.hpp>
#include <array>

namespace xo {
    namespace mm {
        class DX1Collector;

        /** @brief container to hold gc-aware objects for X1 collector
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
            explicit GCObjectStore(const GCObjectStoreConfig & cfg);

            const GCObjectStoreConfig & config() const noexcept { return config_; }

            const ObjectTypeTable * get_object_types() const noexcept { return &object_types_; }
            const DArena * get_space(role r, Generation g) const noexcept { return space_[r][g]; }
            DArena * get_space(role r, Generation g) noexcept { return space_[r][g]; }
            DArena * from_space(Generation g) noexcept { return get_space(role::from_space(), g); }
            DArena * to_space(Generation g) noexcept { return get_space(role::to_space(), g); }
            DArena * new_space() noexcept { return to_space(Generation{0}); }

            /** true iff type with id @p tseq has known metadata
             *  (i.e. has appeared in preceding call to install_type
             *   for this collector)
             **/
            bool is_type_installed(typeseq tseq) const noexcept;

            /** lookup interface from type sequence
             *  (can use tseq = typeseq::id<T>() for type T)
             **/
            const AGCObject * lookup_type(typeseq tseq) const noexcept;

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

            /** true iff address @p addr allocated from this collector
             *  in role @p r (according to current GC state)
             **/
            bool contains(role r, const void * addr) const noexcept;

            /** true iff address @p addr allocated from this collector and currently live
             *  in role @p r (according to current GC state)
             *
             *  (i.e. in [lo,free) for an arena)
             **/
            bool contains_allocated(role r, const void * addr) const noexcept;

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

            /** snap checkpoint containing allocator state
             *  use to detect forwarding activity after visiting objects
             **/
            GCMoveCheckpoint snap_move_checkpoint(Generation upto);

            /** Register object type with this collector.
             *  Provides shallow copy and pointer forwarding for instances of this
             *  type.
             **/
            bool install_type(const AGCObject & meta) noexcept;

            /** traverse objects allocated after @p ckp, to make sure their children
             *  are forwarded. Repeat until traverse doesn't find any unforwarded children.
             *
             *  1. Breadth-first implementation, bad for memory locality
             *  2. Need @p gc for per-object-type forward_children api
             **/
            void _forward_children_until_fixpoint(DX1Collector * gc,
                                                  Generation upto,
                                                  const GCMoveCheckpoint & gray_lo_v);

            /** true iff {@p alloc_hdr, @p object_data} should move for
             *  a collection of all generations strictly younger than @p upto.
             *
             *  Require: runstate_.is_running()
             **/
            bool _check_move_policy(header_type alloc_hdr,
                                    void * gco_data,
                                    Generation upto) const noexcept;

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

            /** configure @ref object_types_, using @p page_z **/
            void _init_object_types(std::size_t page_z);

            /** auxiliary init function **/
            void _init_space();

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

        };

    } /*namespace mm*/
} /*namespace xo*/

/* end GCObjectStore.hpp */
