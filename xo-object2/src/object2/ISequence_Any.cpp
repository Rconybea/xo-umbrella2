/** @file ISequence_Any.cpp
 *
 **/

#include "sequence/ISequence_Any.hpp"
#include <iostream>

namespace xo {
namespace scm {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
ISequence_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " ISequence_Any method"
              << std::endl;
    std::terminate();
}

typeseq
ISequence_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
ISequence_Any::_valid
  = valid_facet_implementation<ASequence, ISequence_Any>();

// nonconst methods


} /*namespace scm*/
} /*namespace xo*/

/* end ISequence_Any.cpp */