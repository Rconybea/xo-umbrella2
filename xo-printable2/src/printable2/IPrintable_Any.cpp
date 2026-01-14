/** @file IPrintable_Any.cpp
 *
 **/

#include "detail/IPrintable_Any.hpp"
#include <iostream>

namespace xo {
namespace print {

using xo::facet::DVariantPlaceholder;
using xo::facet::typeseq;
using xo::facet::valid_facet_implementation;

void
IPrintable_Any::_fatal()
{
    /* control here on uninitialized IAllocator_Any.
     * Initialized instance will have specific implementation type
     */
    std::cerr << "fatal"
              << ": attempt to call uninitialized"
              << " IPrintable_Any method"
              << std::endl;
    std::terminate();
}

typeseq
IPrintable_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

bool
IPrintable_Any::_valid
  = valid_facet_implementation<APrintable, IPrintable_Any>();

// nonconst methods


} /*namespace print*/
} /*namespace xo*/

/* end IPrintable_Any.cpp */