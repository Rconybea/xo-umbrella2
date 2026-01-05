/** @file DFloat.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>

namespace xo {
    namespace scm {
        struct DFloat {
            using AAllocator = xo::mm::AAllocator;
            using ppindentinfo = xo::print::ppindentinfo;

            explicit DFloat(double x) : value_{x} {}

            /** allocate boxed value @p x using memory from @p mm **/
            static DFloat * make(obj<AAllocator> mm,
                                 double x);

            double value() const noexcept { return value_; }

            bool pretty(const ppindentinfo & ppii) const;

            operator double() const noexcept { return value_; }

        private:

            /** boxed floating-oint value **/
            double value_;
        };
    } /*nmaespace scm*/
} /*namespace xo*/

/* end DFloat.hpp */
