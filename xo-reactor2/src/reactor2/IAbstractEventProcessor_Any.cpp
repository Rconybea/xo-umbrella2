/** @file IAbstractEventProcessor_Any.cpp
 *
 **/

#include "detail/IAbstractEventProcessor_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace reactor {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IAbstractEventProcessor_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IAbstractEventProcessor_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IAbstractEventProcessor_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IAbstractEventProcessor_Any::_valid
  = valid_facet_implementation<AAbstractEventProcessor, IAbstractEventProcessor_Any>();

// nonconst methods

auto
IAbstractEventProcessor_Any::set_name(Opaque, const DString *)  noexcept -> void
{
    _fatal();
}


} /*namespace reactor*/
} /*namespace xo*/

/* end IAbstractEventProcessor_Any.cpp */
