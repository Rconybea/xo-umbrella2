/** @file IAllocator_DX1Collector.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 *
 *  See also ICollector_DX1Collector.cpp for collector facet
 **/

#include "gc/IAllocator_DX1Collector.hpp"

namespace xo {
    using std::size_t;

    namespace mm {
        using value_type = IAllocator_DX1Collector::value_type;

        std::string_view
        IAllocator_DX1Collector::name(const DX1Collector & d) noexcept
        {
            return d.config_.name_;
        }

        auto
        IAllocator_DX1Collector::reserved(const DX1Collector & d) noexcept -> size_type
        {
            return d.reserved_total();
        }

        auto
        IAllocator_DX1Collector::size(const DX1Collector & d) noexcept -> size_type
        {
            return d.size_total();
        }

        auto
        IAllocator_DX1Collector::committed(const DX1Collector & d) noexcept -> size_type
        {
            return d.committed_total();
        }

        auto
        IAllocator_DX1Collector::available(const DX1Collector & d) noexcept -> size_type
        {
            return d.available_total();
        }

        auto
        IAllocator_DX1Collector::allocated(const DX1Collector & d) noexcept -> size_type
        {
            return d.allocated_total();
        }

        bool
        IAllocator_DX1Collector::contains(const DX1Collector & d, const void * addr) noexcept
        {
            return d.contains(role::to_space(), addr);
        }

        AllocError
        IAllocator_DX1Collector::last_error(const DX1Collector & d) noexcept
        {
            return d.last_error();
        }

        auto
        IAllocator_DX1Collector::alloc(DX1Collector & d, size_type z) noexcept -> value_type
        {
            return d.alloc(z);
        }

        auto
        IAllocator_DX1Collector::super_alloc(DX1Collector & d, size_type z) noexcept -> value_type
        {
            return d.super_alloc(z);
        }

        auto
        IAllocator_DX1Collector::sub_alloc(DX1Collector & d, size_type z, bool complete) noexcept -> value_type
        {
            return d.sub_alloc(z, complete);
        }

        bool
        IAllocator_DX1Collector::expand(DX1Collector & d, size_type z) noexcept
        {
            return d.expand(z);
        }

        AllocInfo
        IAllocator_DX1Collector::alloc_info(DX1Collector & d, value_type mem) noexcept
        {
            return d.alloc_info(mem);
        }

        void
        IAllocator_DX1Collector::clear(DX1Collector & d)
        {
            d.clear();
        }

        void
        IAllocator_DX1Collector::destruct_data(DX1Collector & d)
        {
            d.~DX1Collector();
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_DX1Collector.cpp */
