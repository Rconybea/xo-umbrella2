/** @file type_keys.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "type_keys.hpp"
#include "JsonPpSink.hpp"

namespace xo {
    namespace json {
        void
        type_keys::write(JsonPpSink & json) const
        {
            json.key("_canonical_type_").string(canonical_)
                .key("_short_type_").string(short_);
        }
    } /*namespace json*/
} /*namespace xo*/

/* end type_keys.cpp */
