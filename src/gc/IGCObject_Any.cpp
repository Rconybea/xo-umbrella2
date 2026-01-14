/** @file IGCObject_Any.cpp
 *
 **/

#include "detail/IGCObject_Any.hpp"
#include <iostream>

namespace xo {
namespace mm {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IGCObject_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IGCObject_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IGCObject_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IGCObject_Any::_valid
  = valid_facet_implementation<AGCObject, IGCObject_Any>();

// nonconst methods

auto
IGCObject_Any::forward_children(Opaque, obj<ACollector>)  const  noexcept -> size_type
{
    _fatal();
}


} /*namespace mm*/
} /*namespace xo*/

/* end IGCObject_Any.cpp */