/** @file type_keys.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "type_keys.hpp"
#include "json_quoted.hpp"

namespace xo {

    namespace json {
        std::ostream &
        operator<<(std::ostream & os, type_keys const & k)
        {
            os << json_quot("_canonical_type_") << ": " << json_quot(k.canonical_)
               << ", " << json_quot("_short_type_") << ": " << json_quot(k.short_);
            return os;
        }
    } /*namespace json*/
} /*namespace xo*/

/* end type_keys.cpp */
