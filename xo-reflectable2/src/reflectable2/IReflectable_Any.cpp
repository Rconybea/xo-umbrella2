/** @file IReflectable_Any.cpp
 *
 **/

#include "detail/IReflectable_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace reflect {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IReflectable_Any::_fatal()
{
    /* control here on uninitialized IReflectable_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IReflectable_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IReflectable_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IReflectable_Any::_valid
  = valid_facet_implementation<AReflectable, IReflectable_Any>();

// nonconst methods


} /*namespace reflect*/
} /*namespace xo*/

/* end IReflectable_Any.cpp */
