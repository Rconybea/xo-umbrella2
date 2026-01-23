/** @file DBoolean.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/gc/Collector.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <xo/facet/obj.hpp>
#include <cstdint>

namespace xo {
    namespace scm {
        struct DBoolean {
            using AAllocator = xo::mm::AAllocator;
            using ACollector = xo::mm::ACollector;
            using ppindentinfo = xo::print::ppindentinfo;
            using value_type = long;

            explicit DBoolean(bool x) : value_{x} {}

            /** will likely want this to default to ANumeric, once we have it **/
            template <typename AFacet>
            static obj<AFacet, DBoolean> box(obj<AAllocator> mm, bool x);

            /** allocate boxed value @p x using memory from @p mm **/
            static DBoolean * _box(obj<AAllocator> mm, bool x);

            bool value() const noexcept { return value_; }

            bool pretty(const ppindentinfo & ppii) const;

            operator bool() const noexcept { return value_; }

            // GCObject facet

            std::size_t shallow_size() const noexcept;
            DBoolean * shallow_copy(obj<AAllocator> mm) const noexcept;
            std::size_t forward_children(obj<ACollector> gc) noexcept;

        private:
            /** boxed boolean value **/
            bool value_;
        };

        template <typename AFacet>
        obj<AFacet, DBoolean>
        DBoolean::box(obj<AAllocator> mm, bool x) {
            return obj<AFacet,DBoolean>(_box(mm, x));
        }

    } /*nmaespace obj*/
} /*namespace xo*/

/* end DBoolean.hpp */
