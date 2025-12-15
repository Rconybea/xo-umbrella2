/** @file IAllocator_DX1Collector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "alloc2/alloc/Allocator.hpp"
#include "alloc2/alloc/IAllocator_Xfer.hpp"
#include "DX1Collector.hpp"

namespace xo {
    namespace mm { struct IAllocator_DX1Collector; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AAllocator,
                                   xo::mm::DX1Collector>
        {
            using ImplType = xo::mm::IAllocator_Xfer<xo::mm::DX1Collector,
                                                     xo::mm::IAllocator_DX1Collector>;
        };
    }

    namespace mm {
        /* changes here coordinate with
         *  AAllocator      AAllocator.hpp
         *  IAllocator_Any  IAllocator_Any.hpp
         *  IAllocator_Xfer IAllocator_Xfer.hpp
         *  RAllocator      RCollector.hpp
         */
        struct IAllocator_DX1Collector {
            using size_type = std::size_t;
            using value_type = std::byte *;

            // todo: available()

            static std::string_view name(const DX1Collector &) noexcept;
            /** reserved memory across all {roles, generations}  **/
            static size_type reserved(const DX1Collector &) noexcept;
            static size_type size(const DX1Collector &) noexcept;
            /** committed size accross all {roles, generations} **/
            static size_type committed(const DX1Collector &) noexcept;
            /** available (committed but unused) space across all {roles, generations} **/
            static size_type available(const DX1Collector &) noexcept;
            /** space used by @p d across all {roles, generations}. **/
            static size_type allocated(const DX1Collector &) noexcept;
            /** true iff address @p p comes from collector @p d **/
            static bool contains(const DX1Collector & d, const void * p) noexcept;
            /** report last error, if any, for collector @p d **/
            static AllocatorError last_error(const DX1Collector &) noexcept;

            /** expand gen0 spaces (both from-space and to-space) **/
            static bool expand(DX1Collector & d, size_type z) noexcept;

            /** always alloc in gen0 to-space **//
            static value_type alloc(DX1Collector & d, size_type z);
            static value_type super_alloc(DX1Collector & d, size_type z);
            static value_type sub_alloc(DX1Collector & d, size_type z, bool complete);

            /** reset to empty state; clears all generations **/
            static void clear(DX1Collector & d);
            /** invoke destructor **/
            static void destruct_data(DX1Collector & d);
        };

    } /*namespace mm*/
} /*namespace xo*/


/* end IAllocator_DX1Collector.hpp */
