/** @file AAllocator.cpp **/

#include "xo/alloc2/AAllocator.hpp"

namespace xo {
    using xo::facet::DVariantPlaceholder;
    using xo::facet::typeseq;
    using xo::facet::valid_facet_implementation;

    namespace mm {
        int32_t
        IAllocator_Any::s_typeseq = typeseq::id<DVariantPlaceholder>;

        bool
        IAllocator_Any::_valid = valid_facet_implementation<AAllocator, IAllocator_Any>;
    }
}

/* end AAlocator.cpp */
