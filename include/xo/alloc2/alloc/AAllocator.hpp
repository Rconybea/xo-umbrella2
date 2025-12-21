/** @file AAllocator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AllocError.hpp"
#include "AllocInfo.hpp"
//#include "AllocIterator.hpp"
#include "AllocRange.hpp"
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>
#include <string>

namespace xo {
    namespace mm {
        using Copaque = const void *;
        using Opaque = void *;

        // see DArena.hpp
        struct DArena;

        /** @class AAllocator
         *  @brief Abstract facet for allocation
         *
         **/
        struct AAllocator {
            /** @defgroup mm-allocator-type-traits allocator type traits **/
            ///@{
            /** @brief type used for allocation amounts **/
            using size_type = std::size_t;
            /** @brief type used for allocation responses **/
            using value_type = std::byte *;
            /** object header, if configured **/
            using header_type = std::uint64_t;
            /** iterator range. These are forward iterators over allocs **/
            using range_type = AllocRange;
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

            /** RTTI: unique id# for actual runtime data representation **/
            virtual int32_t _typeseq() const noexcept = 0;
            /** optional name for allocator @p d
             *  Labeling, for diagnostics.
             **/
            virtual std::string_view name(Copaque d) const noexcept = 0;
            /** reserved size in bytes for allocator @p d.
             *  Includes committed + uncommitted memory.
             *  Cannot be increased.
             **/
            virtual size_type reserved(Copaque d) const noexcept = 0;
            /** Synonym for @ref committed.
             *  Can increase on @ref alloc
             **/
            virtual size_type size(Copaque d) const noexcept = 0;
            /** committed size (physical addresses obtained)
             *  for allocator @p d.
             *  @ref alloc may auto-increase this
             **/
            virtual size_type committed(Copaque d) const noexcept = 0;
            /** unallocated (but committed) size in bytes for allocator @p d **/
            virtual size_type available(Copaque d) const noexcept = 0;
            /** allocated (i.e. in-use) amount in bytes for allocator @p d **/
            virtual size_type allocated(Copaque d) const noexcept = 0;
            /** true iff allocator @p d is responsible for memory at address @p p.
             **/
            virtual bool contains(Copaque d, const void * p) const noexcept = 0;
            /** report last error **/
            virtual AllocError last_error(Copaque d) const noexcept = 0;
            /** fetch alloc info: given memory @p mem previously obtained
             *  from {@ref alloc, @ref super_alloc}, get {tseq, age, size} details
             *  for that allocation.
             *
             *  Non-const @p d because may stash error details
             **/
            virtual AllocInfo alloc_info(Copaque d, value_type mem) const noexcept = 0;
            /** Ideally we want to control allocator for iterator here.
             *  Awkward to supply to compiler since we don't have obj<AAllocator> yet.
             *  OTOH iteration over allocs is a super-niche feature.
             *
             *  Rejected alternatives:
             *  - put begin/end in separate interface. e.g. extend AAllocator
             *  - layer of indirection: begin/end return iterator factory.
             *    Then allocator can be passed to iterator factory separately.
             *    Helps because factory can be static
             *  - abandon allocator support in this case. Instead will need to
             *    reinstate uvt<AAllocIterator> (unique variant), use heap
             *
             *  @p mm is allocator for resulting iterator range
             **/
            virtual range_type alloc_range(Copaque d, DArena & mm) const noexcept = 0;

            /** expand committed space in arena @p d
             *  to size at least @p z
             *  In practice will round up to a multiple of hugepage size (2MB)
             **/
            virtual bool expand(Opaque d, std::size_t z) const noexcept = 0;
            /** allocate @p z bytes of memory from allocator @p d. **/
            virtual value_type alloc(Opaque d, size_type z) const = 0;
            /** like @ref alloc, but follow with one or more consecutive
             *  @ref sub_alloc() calls. This sequence of allocs will share
             *  the initial allocation header.
             **/
            virtual value_type super_alloc(Opaque d, size_type z) const = 0;
            /** follow a preceding @ref super_alloc call with additional
             *  subsidiary allocs that share the same object header.
             *  Must finish sequence with exactly one sub_alloc call
             *  with @p complete_flag set. This sub_alloc call may have
             *  zero @p z
             **/
            virtual value_type sub_alloc(Opaque d, size_type z, bool complete_flag) const = 0;
            /** reset allocator @p d to empty state **/
            virtual void clear(Opaque d) const = 0;
            /** destruct allocator @p d **/
            virtual void destruct_data(Opaque d) const = 0;

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
