/** @file DInteger.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <cstdint>

namespace xo {
    namespace scm {
        struct DInteger {
            using AAllocator = xo::mm::AAllocator;
            using ppindentinfo = xo::print::ppindentinfo;

            explicit DInteger(long x) : value_{x} {}

            /** allocate boxed value @p x using memory from @p mm **/
            static DInteger * make(obj<AAllocator> mm,
                                   long x);

            double value() const noexcept { return value_; }

            bool pretty(const ppindentinfo & ppii) const;

            operator long() const noexcept { return value_; }

        private:
            /** boxed integer value **/
            long value_;
        };
    } /*nmaespace obj*/
} /*namespace xo*/

/* end DInteger.hpp */
