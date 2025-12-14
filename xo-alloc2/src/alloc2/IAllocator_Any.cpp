/** @file IAllocator_Any.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "alloc/IAllocator_Any.hpp"
#include <iostream>

namespace xo {
    using xo::facet::DVariantPlaceholder;
    using xo::facet::typeseq;
    using xo::facet::valid_facet_implementation;

    namespace mm {

        void
        IAllocator_Any::_fatal() {
            /* control here on uninitialized IAllocator_Any.
             * Initialized instance will have specific implementation type
             * e.g. IAllocator_Xfer<DArena>
             */

            std::cerr << "fatal"
                      << ": attempt to call uninitialized"
                      << " IAllocator_Any method"
                      << std::endl;
            std::terminate();
        }

        int32_t
        IAllocator_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

        bool
        IAllocator_Any::_valid = valid_facet_implementation<AAllocator, IAllocator_Any>();

    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_Any.cpp */
