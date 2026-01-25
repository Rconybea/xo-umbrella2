/** @file GCObjectConversion_DFloat.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DFloat.hpp"
#include "number/IGCObject_DFloat.hpp"
#include <xo/gc/GCObjectConversion.hpp>

namespace xo {
    namespace scm {

        template <>
        struct GCObjectConversion<double> {
            static_assert(std::is_same_v<double, DFloat::value_type>);

            using AGCObject = xo::mm::AGCObject;
            using AAllocator = xo::mm::AAllocator;

            static obj<AGCObject> to_gco(obj<AAllocator> mm, const double & x);
            static double from_gco(obj<AAllocator> mm, obj<AGCObject> gco);
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end GCObjectConversion_DFloat.hpp */
