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

        /** @class AAllocator
         *  @brief Abstract facet for allocation
         *
         **/
        struct AAllocator {
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
            virtual int32_t _typeseq() const = 0;
            /** optional name for allocator @p d
             *  Labeling, for diagnostics.
             **/
            virtual const std::string & name(Copaque d) const = 0;
            /** reserved size in bytes for allocator @p d.
             *  Includes committed + uncommitted memory.
             *  Cannot be increased.
             **/
            virtual std::size_t reserved(Copaque d) const = 0;
            /** allocator size in bytes (up to reserved limit)
             *  for allocator @p d.
             *  Includes all committed memory.
             *  Can increase on @ref alloc
             **/
            virtual std::size_t size(Copaque d) const = 0;
            /** committed size (physical addresses obtained)
             *  for allocator @p d.
             **/
            virtual std::size_t committed(Copaque d) const = 0;
            /** true iff allocator @p d is responsible for memory at address @p p.
             **/
            virtual bool contains(Copaque d, const void * p) const = 0;

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
