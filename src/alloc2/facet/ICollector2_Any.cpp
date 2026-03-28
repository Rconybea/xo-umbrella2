/** @file ICollector2_Any.cpp
 *
 **/

#include "gc/ICollector2_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace mm {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
ICollector2_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " ICollector2_Any method"
              << std::endl;
    std::terminate();
}

typeseq
ICollector2_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
ICollector2_Any::_valid
  = valid_facet_implementation<ACollector2, ICollector2_Any>();

// nonconst methods

auto
ICollector2_Any::install_type(Opaque, const AGCObject &)  -> bool
{
    _fatal();
}

auto
ICollector2_Any::add_gc_root_poly(Opaque, obj<AGCObject> *)  -> void
{
    _fatal();
}

auto
ICollector2_Any::remove_gc_root_poly(Opaque, obj<AGCObject> *)  -> void
{
    _fatal();
}

auto
ICollector2_Any::request_gc(Opaque, Generation)  -> void
{
    _fatal();
}

auto
ICollector2_Any::assign_member(Opaque, void *, obj<AGCObject> *, obj<AGCObject> &)  -> void
{
    _fatal();
}

auto
ICollector2_Any::forward_inplace(Opaque, AGCObject *, void **)  -> void
{
    _fatal();
}


} /*namespace mm*/
} /*namespace xo*/

/* end ICollector2_Any.cpp */
