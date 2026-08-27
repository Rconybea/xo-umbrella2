/** @file IEventSink_Any.cpp
 *
 **/

#include "detail/IEventSink_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace reactor {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IEventSink_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IEventSink_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IEventSink_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IEventSink_Any::_valid
  = valid_facet_implementation<AEventSink, IEventSink_Any>();

// nonconst methods

auto
IEventSink_Any::notify_ev_tp(Opaque, const TaggedPtr &)  -> void
{
    _fatal();
}


} /*namespace reactor*/
} /*namespace xo*/

/* end IEventSink_Any.cpp */
