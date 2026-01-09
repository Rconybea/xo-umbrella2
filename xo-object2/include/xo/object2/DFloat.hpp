/** @file DFloat.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/obj.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>

namespace xo {
    namespace scm {
        struct DFloat {
            using AAllocator = xo::mm::AAllocator;
            using ppindentinfo = xo::print::ppindentinfo;

            explicit DFloat(double x) : value_{x} {}

            /** probably want default = ANumeric, once we introduce it **/
            template <typename AFacet>
            static obj<AFacet,DFloat> box(obj<AAllocator> mm, double x);

            /** allocate boxed value @p x using memory from @p mm **/
            static DFloat * _box(obj<AAllocator> mm, double x);

            double value() const noexcept { return value_; }

            bool pretty(const ppindentinfo & ppii) const;

            operator double() const noexcept { return value_; }

        private:

            /** boxed floating-oint value **/
            double value_;
        };

        template <typename AFacet>
        obj<AFacet,DFloat>
        DFloat::box(obj<AAllocator> mm, double x) {
            return obj<AFacet,DFloat>(DFloat::_box(mm, x));
        }
    } /*nmaespace scm*/
} /*namespace xo*/

/* end DFloat.hpp */
