/** @file type_keys.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "type_keys.hpp"
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/quoted.hpp>

namespace xo {
    namespace pp {
        void
        Prettifier<xo::json::type_keys>::print(PpSink & sink, xo::json::type_keys const & k)
        {
            sink(quot("_canonical_type_"), ": ", quot(k.canonical_),
                 ", ", quot("_short_type_"), ": ", quot(k.short_));
        }
    } /*namespace pp*/
} /*namespace xo*/

/* end type_keys.cpp */
