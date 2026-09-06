/** @file ITop_Any.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "top/ITop_Any.hpp"
#include <exception>
#include <iostream>

namespace xo::facet {

    // LCOV_EXCL_START

    void
    ITop_Any::_fatal()
    {
        /* control here on uninitialized ITop_Any.
         * Initialized instance will have specific implementation type
         */

        std::cerr << "fatal"
        << ": attempt to call uninitialized"
        << " ITop_Any method"
        << std::endl;

        std::terminate();
    }

    // LCOV_EXCL_STOP

    typeseq
    ITop_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

    bool
    ITop_Any::_valid = valid_facet_implementation<ATop, ITop_Any>();

} /*namespace xo::facet*/

/* end ITop_Any.cpp */
