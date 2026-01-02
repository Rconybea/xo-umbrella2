/** @file IAllocIterator_Any.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "alloc/IAllocIterator_Any.hpp"
#include <iostream>

namespace xo {
    namespace mm {
        using xo::facet::DVariantPlaceholder;
        using xo::facet::typeseq;
        using xo::facet::valid_facet_implementation;

        void
        IAllocIterator_Any::_fatal() {
            /* control here on uninitialized IAllocator_Any.
             * Initialized instance will have specific implementation type
             * e.g. IAllocator_Xfer<DArena>
             */

            std::cerr << "fatal"
                      << ": attempt to call uninitialized"
                      << " IAllocIterator_Any method"
                      << std::endl;
            std::terminate();
        }

        typeseq
        IAllocIterator_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

        bool
        IAllocIterator_Any::_valid = valid_facet_implementation<AAllocIterator,
                                                                IAllocIterator_Any>();


    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocIterator_Any.cpp */
