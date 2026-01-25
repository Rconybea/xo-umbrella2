/** @file IProcedure_Any.cpp
 *
 **/

#include "detail/IProcedure_Any.hpp"
#include <iostream>

namespace xo {
namespace scm {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IProcedure_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IProcedure_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IProcedure_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IProcedure_Any::_valid
  = valid_facet_implementation<AProcedure, IProcedure_Any>();

// nonconst methods

auto
IProcedure_Any::apply_nocheck(Opaque, obj<AAllocator>, const DArray *)  -> obj<AGCObject>
{
    _fatal();
}


} /*namespace scm*/
} /*namespace xo*/

/* end IProcedure_Any.cpp */