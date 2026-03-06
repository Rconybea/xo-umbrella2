/** @file IType_Any.cpp
 *
 **/

#include "type/IType_Any.hpp"
#include <iostream>
#include <exception>

namespace xo {
namespace scm {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IType_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IType_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IType_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IType_Any::_valid
  = valid_facet_implementation<AType, IType_Any>();

// nonconst methods


} /*namespace scm*/
} /*namespace xo*/

/* end IType_Any.cpp */
