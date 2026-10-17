/** @file type_keys.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "type_keys.hpp"
#include "json_layout.hpp"
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/quoted.hpp>

namespace xo {
    using xo::json::type_keys;
    using xo::json::json_sep;

    namespace pp {
        void
        Prettifier<type_keys>::print(PpSink & sink, type_keys const & k)
        {
            sink(quot("_canonical_type_"), ": ", quot(k.canonical_),
                 json_sep(), quot("_short_type_"), ": ", quot(k.short_));
        }
    } /*namespace pp*/
} /*namespace xo*/

/* end type_keys.cpp */
