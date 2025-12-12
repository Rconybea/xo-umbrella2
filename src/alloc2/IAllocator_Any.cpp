/** @file IAllocator_Any.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "IAllocator_Any.hpp"

namespace xo {
    using xo::facet::DVariantPlaceholder;
    using xo::facet::typeseq;
    using xo::facet::valid_facet_implementation;

    namespace mm {

        int32_t
        IAllocator_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

        bool
        IAllocator_Any::_valid = valid_facet_implementation<AAllocator, IAllocator_Any>();

    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_Any.cpp */
