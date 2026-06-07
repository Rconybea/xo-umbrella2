/** @file ICollector_Any.cpp
 *
 **/

#include "gc/ICollector_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace mm {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
ICollector_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " ICollector_Any method"
              << std::endl;
    std::terminate();
}

typeseq
ICollector_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
ICollector_Any::_valid
  = valid_facet_implementation<ACollector, ICollector_Any>();

// nonconst methods

auto
ICollector_Any::install_type(Opaque, const AGCObject &)  -> bool
{
    _fatal();
}

auto
ICollector_Any::add_gc_root_poly(Opaque, obj<AGCObject> *)  -> void
{
    _fatal();
}

auto
ICollector_Any::remove_gc_root_poly(Opaque, obj<AGCObject> *)  -> void
{
    _fatal();
}

auto
ICollector_Any::request_gc(Opaque, Generation)  -> void
{
    _fatal();
}

auto
ICollector_Any::assign_member(Opaque, void *, obj<AGCObject> *, obj<AGCObject> &)  -> void
{
    _fatal();
}

auto
ICollector_Any::alloc_copy(Opaque, std::byte *)  -> void *
{
    _fatal();
}


} /*namespace mm*/
} /*namespace xo*/

/* end ICollector_Any.cpp */
