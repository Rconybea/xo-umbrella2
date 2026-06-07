/** @file IGCObjectVisitor_Any.cpp
 *
 **/

#include "gc/IGCObjectVisitor_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace mm {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IGCObjectVisitor_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IGCObjectVisitor_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IGCObjectVisitor_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IGCObjectVisitor_Any::_valid
  = valid_facet_implementation<AGCObjectVisitor, IGCObjectVisitor_Any>();

// nonconst methods

auto
IGCObjectVisitor_Any::alloc_copy(Opaque, std::byte *)  const -> void *
{
    _fatal();
}

auto
IGCObjectVisitor_Any::visit_child(Opaque, VisitReason, AGCObject *, void **)  const  noexcept -> void
{
    _fatal();
}


} /*namespace mm*/
} /*namespace xo*/

/* end IGCObjectVisitor_Any.cpp */
