/** @file GCObjectConversion.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/gc/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>

namespace xo {
    namespace scm {
        /** @brief compile-time conversion obj<AGCObject> <-> T
         *
         *  Specialize for each T that participates in conversion.
         *  Methods here aren't implemented
         **/
        template <typename T>
        struct GCObjectConversion {
            using AGCObject = xo::mm::AGCObject;
            using AAllocator = xo::mm::AAllocator;

            static obj<AGCObject> to_gco(obj<AAllocator> mm, const T & x);
            static T from_gco(obj<AAllocator> mm, obj<AGCObject> gco);
        };

    } /*namespace scm */
} /*namespace xo*/

/* end GCObjectConversion.hpp */
