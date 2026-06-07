/** @file IGCObject_Any.cpp
 *
 **/

#include "gc/IGCObject_Any.hpp"
#include <iostream>
#include <exception>

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
IGCObject_Any::gco_shallow_move(Opaque, obj<AGCObjectVisitor>)  const  noexcept -> Opaque
{
    _fatal();
}

auto
IGCObject_Any::visit_gco_children(Opaque, VisitReason, obj<AGCObjectVisitor>)  const  noexcept -> void
{
    _fatal();
}


} /*namespace mm*/
} /*namespace xo*/

/* end IGCObject_Any.cpp */
