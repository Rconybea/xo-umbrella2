/** @file IRuntimeContext_Any.cpp
 *
 **/

#include "detail/IRuntimeContext_Any.hpp"
#include <iostream>

namespace xo {
namespace scm {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IRuntimeContext_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IRuntimeContext_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IRuntimeContext_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IRuntimeContext_Any::_valid
  = valid_facet_implementation<ARuntimeContext, IRuntimeContext_Any>();

// nonconst methods


} /*namespace scm*/
} /*namespace xo*/

/* end IRuntimeContext_Any.cpp */
