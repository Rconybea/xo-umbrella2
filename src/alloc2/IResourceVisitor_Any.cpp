/** @file IResourceVisitor_Any.cpp
 *
 **/

#include "visitor/IResourceVisitor_Any.hpp"
#include <iostream>

namespace xo {
namespace mm {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IResourceVisitor_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IResourceVisitor_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IResourceVisitor_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IResourceVisitor_Any::_valid
  = valid_facet_implementation<AResourceVisitor, IResourceVisitor_Any>();

// nonconst methods


} /*namespace mm*/
} /*namespace xo*/

/* end IResourceVisitor_Any.cpp */
