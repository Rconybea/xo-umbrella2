/** @file IHashable_Any.cpp
 *
 **/

#include "detail/IHashable_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace hashable {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IHashable_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IHashable_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IHashable_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IHashable_Any::_valid
  = valid_facet_implementation<AHashable, IHashable_Any>();

// nonconst methods


} /*namespace hashable*/
} /*namespace xo*/

/* end IHashable_Any.cpp */
