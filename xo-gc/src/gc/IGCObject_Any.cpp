/** @file IGCObject_Any.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "gcobject/IGCObject_Any.hpp"
#include <iostream>

namespace xo {
    using xo::facet::DVariantPlaceholder;
    using xo::facet::typeseq;
    using xo::facet::valid_facet_implementation;

    namespace mm {

        void
        IGCObject_Any::_fatal() {
            std::cerr << "fatal"
                      << ": attempt to call uninitialized"
                      << " IGCObject_Any method"
                      << std::endl;
            std::terminate();
        }

        int32_t
        IGCObject_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

        bool
        IGCObject_Any::_valid = valid_facet_implementation<AGCObject, IGCObject_Any>();

    } /*namespace mm*/
} /*namespace xo*/

/* end IGCObject_Any.cpp */
