/** @file AAllocator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "xo/facet/facet_implementation.hpp"
#include "xo/facet/typeseq.hpp"
#include <string>

namespace xo {
    namespace mm {
        using Copaque = const void *;
        using Opaque = void *;

        enum class error : int32_t {
            /** sentinel **/
            invalid = -1,
            /** not an error **/
            none,
            /** reserved size exhauged **/
            reserve_exhausted,
            /** unable to commit (i.e. mprotect failure) **/
            commit_failed,
        };

        struct AllocatorError {
            using size_type = std::size_t;
            using value_type = std::byte*;

            AllocatorError() = default;
            explicit AllocatorError(error err) : error_{err} {}
            AllocatorError(error err,
                           size_type req_z,
                           size_type com_z,
                           size_type rsv_z) : error_{err},
                                              request_z_{req_z},
                                              committed_z_{com_z},
                                              reserved_z_{rsv_z} {}

            /** error code **/
            error error_ = error::none;

            /** reqeust size assoc'd with errror **/
            size_type request_z_ = 0;
            /** committed allocator memory at time of error **/
            size_type committed_z_ = 0;
            /** reserved allocator memory at time of error **/
            size_type reserved_z_ = 0;
        };

        /** @class AAllocator
         *  @brief Abstract facet for allocation
         *
         **/
        struct AAllocator {
            /** @defgroup mm-allocator-type-traits allocator type traits **/
            ///@{
            /** @brief type used for allocation amounts **/
            using size_type = std::size_t;
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
            virtual const std::string & name(Copaque d) const noexcept = 0;
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

            /** expand committed space in arena @p d
             *  to size at least @p z
             *  In practice will round up to a multiple of hugepage size (2MB)
             **/
            virtual bool expand(Opaque d, std::size_t z) const noexcept = 0;
            /** allocate @p z bytes of memory from allocator @p d. **/
            virtual std::byte * alloc(Opaque d, std::size_t z) const = 0;
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
