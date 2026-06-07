/** @file AAllocator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/arena/MemorySizeInfo.hpp>
#include <xo/arena/AllocError.hpp>
#include "AllocInfo.hpp"
#include "AllocRange.hpp"
#include "typeseq.hpp"
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>
#include <string>

namespace xo {
    namespace mm {
        using Copaque = const void *;
        using Opaque = void *;

        class AGCObject;      // see AGCObject.hpp
        struct DArena;        // see DArena.hpp

        /** @class AAllocator
         *  @brief Abstract facet for allocation
         *
         *  Methods take a opaque data pointer.
         *  Implementations of AAllocator will downcast to a
         *  to some specific representation.
         **/
        class AAllocator {
        public:
            /** @defgroup mm-allocator-type-traits allocator type traits **/
            ///@{
            using AGCObject = xo::mm::AGCObject;
            /** memory size report **/
            using MemorySizeInfo = xo::mm::MemorySizeInfo;
            /** type used for allocation amounts **/
            using size_type = std::size_t;
            /** type used for allocation responses **/
            using value_type = std::byte *;
            /** object header, if configured **/
            using header_type = std::uint64_t;
            /** iterator range. These are forward iterators over allocs **/
            using range_type = AllocRange;
            /** sequence number identifying a datatype **/
            using typeseq = xo::facet::typeseq;
            ///@}

            /*
             *    <----------------------------size-------------------------->
             *    <------------committed-----------><-------uncommitted------>
             *    <--allocated-->
             *
             *    XXXXXXXXXXXXXXX___________________..........................
             *
             *    allocated:   in use
             *    committed:   physical memory obtained
             *    uncommitted: mapped in virtual memory, not backed by memory
             */

            /** @defgroup mm-allocator-methods Allocator methods **/
            ///@{

            /** An uninitialized AAllocator instance will have zero vtable pointer
             *  (per {linux,osx} abi).
             *  Use case for this is narrow.
             *  We go to some lengths to avoid null vtable pointers.
             *  For example obj<AFacet> will have non-null vtable (via IFacet_Any)
             *  with all methods terminating.
             **/
            bool _has_null_vptr() const noexcept {
                return (*reinterpret_cast<const void * const *>(this) == nullptr);
            }
            /** RTTI: unique id# for actual runtime data representation **/
            virtual typeseq _typeseq() const noexcept = 0;
            /** destroy instance @p d. Calls c++ destructor for actual runtime type.
             *  does not recover memory.
             **/
            virtual void _drop(Opaque d) const noexcept = 0;
            /** optional name for allocator @p d .
             *  Allows labeling allocators, for diagnostics/instrumentation.
             **/
            virtual std::string_view name(Copaque d) const noexcept = 0;
            /** reserved size in bytes for allocator @p d.
             *  Includes committed + uncommitted memory.
             **/
            virtual size_type reserved(Copaque d) const noexcept = 0;
            /** Synonym for @ref committed.
             *  Can increase automatically on @ref alloc
             **/
            virtual size_type size(Copaque d) const noexcept = 0;
            /** committed size (physical addresses obtained)
             *  for allocator @p d.
             *  @ref alloc may auto-increase this
             **/
            virtual size_type committed(Copaque d) const noexcept = 0;
            /** unallocated (but committed) size in bytes for allocator @p d.
             *  An alloc request up to this size (including guard / header)
             *  is guaranteed to succeed.
             *  An alloc request of more than this size may still succeed,
             *  if allocator can automatically extend committed memory.
             *  This is the case for the @ref xo::mm::DArena allocator
             **/
            virtual size_type available(Copaque d) const noexcept = 0;
            /** allocated (i.e. currently in-use) amount in bytes for allocator @p d.
             *  Includes alloc headers and guard regions
             **/
            virtual size_type allocated(Copaque d) const noexcept = 0;
            /** call @p fn(i,n,info) for each memory pool owned by this allocator.
             *  Note: using std::function instead of obj<> to avoid leveling trouble
             *  with DArena
             **/
            virtual void visit_pools(Copaque d,
                                     const MemorySizeVisitor & fn) const = 0;
            /** true iff allocator @p d is responsible for memory at address @p p.
             **/
            virtual bool contains(Copaque d, const void * p) const noexcept = 0;
            /** report details of last error for allocator @p d. **/
            virtual AllocError last_error(Copaque d) const noexcept = 0;
            /** fetch alloc info: given memory @p mem previously obtained
             *  from {@ref alloc, @ref super_alloc}, get {tseq, age, size} details
             *  for that allocation.
             *
             *  Non-const @p d because may stash error details
             **/
            virtual AllocInfo alloc_info(Copaque d, value_type mem) const noexcept = 0;
            /**
             *  Create an iterator range for allocator @p d.
             *  An iterator range has begin and end methods, so supports c++ range iteration.
             *  Memory for iterator state will be obtained from @p mm.
             **/
            virtual range_type alloc_range(Copaque d, DArena & mm) const noexcept = 0;

            /** expand committed space in arena @p d
             *  to size at least @p z.
             *  In practice will round up to a multiple of page size (4K) or hugepage size (2MB)
             *  depending on configuration.
             **/
            virtual bool expand(Opaque d, std::size_t z) const noexcept = 0;
            /** attempt to allocate @p z bytes of memory from allocator @p d.
             *  for object with type @p t.
             *  (DX1collector cares about @p t, DArena does not)
             *  If allocation fails returns nullptr. In this case error details may be retrieved
             *  using last error
             **/
            virtual value_type alloc(Opaque d, typeseq t, size_type z) const = 0;
            /** like @ref alloc, but follow with one or more consecutive
             *  @ref sub_alloc() calls. This sequence of allocs will share
             *  the initial allocation header.
             **/
            virtual value_type super_alloc(Opaque d, typeseq t, size_type z) const = 0;
            /** follow a preceding @ref super_alloc call with additional
             *  subsidiary allocs that share the same object header.
             *  Must finish sequence with exactly one sub_alloc call
             *  with @p complete_flag set. This sub_alloc call may have
             *  zero @p z.
             **/
            virtual value_type sub_alloc(Opaque d, size_type z, bool complete_flag) const = 0;
#ifdef OBSOLETE
            /** Allocate copy of an existing object @p src.
             *  Existing object must be contained in @p d.
             *  NOTE: load bearing for copying garbage collector.
             **/
            virtual value_type alloc_copy(Opaque d, value_type src) const = 0;
#endif
            /** reset allocator @p d to empty state. **/
            virtual void clear(Opaque d) const = 0;
            /** assign helper for allocator that may require a write barrier
             *  (for example Collector). Using obj<AGCObject> here causes
             *  include cycle, use spelled out form instead;
             *
             *  For:
             *    obj<AGCObject> lhs_ = rhs
             *  with allocator d, that owns some allocaiton p, would use
             *    mm.barrier_assign_aux(d, p,
             *                          lhs_.iface(), lhs_.opaque_data_addr(),
             *                          rhs.iface(), rhs.opaque_data());
             *  when lhs has known data type:
             *    DRepr * lhs_ = rhs.data();
             *  use
             *    mm.barrier_assign_aux(d, p,
             *                          nullptr, lhs_.opaque_data_addr(),
             *                          rhs.iface(), rhs.opaque_data());
             *  barrier needs to be able to construct complete fop for rhs
             *  to administrate write barrier.
             **/
            virtual void barrier_assign_aux(Opaque d, void * parent,
                                            AGCObject * lhs_iface, void ** lhs_data,
                                            AGCObject * rhs_iface, void * rhs_data) const = 0;

            ///@}
        }; /*AAllocator*/

        // implementation IAllocator_DRepr of AAllocator for state DRepr
        // should provide a specialization:
        //
        //   template <>
        //   struct xo::facet::FacetImplementation<AAllocator, DRepr> {
        //       using ImplType = IAllocator_DRepr;
        //   };
        //
        //   then IAllocator_ImplType<DRepr> --> IAllocator_DRepr
        //
        template <typename DRepr>
        using IAllocator_ImplType = xo::facet::FacetImplType<AAllocator, DRepr>;

// can't we do this with FacetImplementation<AAllocator, DRepr>
//
//        template <typename DRepr>
//        struct IAllocator_Impl {};

//        template <typename DRepr>
//        using IAllocator_ImplType = IAllocator_Impl<DRepr>::ImplType;

    } /*namespace mm*/
} /*namespace xo*/

/* end AAllocator.hpp */
