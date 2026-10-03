/** @file type_keys.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "type_keys.hpp"
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */

namespace xo {
    using xo::pp::quot;

    namespace json {
        std::ostream &
        operator<<(std::ostream & os, type_keys const & k)
        {
            os << quot("_canonical_type_") << ": " << quot(k.canonical_)
               << ", " << quot("_short_type_") << ": " << quot(k.short_);
            return os;
        }
    } /*namespace json*/
} /*namespace xo*/

/* end type_keys.cpp */
