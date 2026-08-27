/** @file IEventSource_Any.cpp
 *
 **/

#include "detail/IEventSource_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace reactor {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IEventSource_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IEventSource_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IEventSource_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IEventSource_Any::_valid
  = valid_facet_implementation<AEventSource, IEventSource_Any>();

// nonconst methods

auto
IEventSource_Any::attach_sink(Opaque, obj<AEventSink>)  -> CallbackId
{
    _fatal();
}

auto
IEventSource_Any::detach_sink(Opaque, CallbackId)  -> void
{
    _fatal();
}

auto
IEventSource_Any::deliver_one(Opaque)  -> uint64_t
{
    _fatal();
}


} /*namespace reactor*/
} /*namespace xo*/

/* end IEventSource_Any.cpp */
