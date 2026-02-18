/** @file GCObjectConversion_DInteger.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DInteger.hpp"
#include "number/IGCObject_DInteger.hpp"
#include <xo/gc/GCObjectConversion.hpp>

namespace xo {
    namespace scm {

        template <>
        struct GCObjectConversion<long> {
            static_assert(std::is_same_v<long, DInteger::value_type>);

            using AGCObject = xo::mm::AGCObject;
            using AAllocator = xo::mm::AAllocator;

            static obj<AGCObject> to_gco(obj<AAllocator> mm, long x);
            static long from_gco(obj<AAllocator> mm, obj<AGCObject> gco);
        };

    }
} /*namespace xo*/

/* end GCObjectConversion_DInteger.hpp */
