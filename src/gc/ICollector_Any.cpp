/** @file ICollector_Any.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "detail/ICollector_Any.hpp"
#include <iostream>

namespace xo {
    using xo::facet::DVariantPlaceholder;
    using xo::facet::typeseq;
    using xo::facet::valid_facet_implementation;

    namespace mm {

        void
        ICollector_Any::_fatal() {
            /* control here on uninitialized ICollector_Any.
             * Initialized instance will have specific implementation type
             * e.g. ICollector_Xfer<DCollector>
             */

            std::cerr << "fatal"
                      << ": attempt to call uninitialized"
                      << " ICollector_Any method"
                      << std::endl;
            std::terminate();
        }

        int32_t
        ICollector_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

        bool
        ICollector_Any::_valid = valid_facet_implementation<ACollector, ICollector_Any>();

    } /*namespace mm*/
} /*namespace xo*/

/** end ICollector_Any.cpp */
