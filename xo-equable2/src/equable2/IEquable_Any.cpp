/** @file IEquable_Any.cpp
 *
 **/

#include "detail/IEquable_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace equable {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IEquable_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IEquable_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IEquable_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IEquable_Any::_valid
  = valid_facet_implementation<AEquable, IEquable_Any>();

// nonconst methods


} /*namespace equable*/
} /*namespace xo*/

/* end IEquable_Any.cpp */
