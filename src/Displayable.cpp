/* @file Displayable.cpp */

#include "refcnt/Displayable.hpp"

namespace xo {
    using xo::tostr;

    namespace ref {
        std::string
        Displayable::display_string() const {
            return tostr(*this);
        } /*display_string*/
    } /*namespace ref*/
} /*namespace xo*/

/* end Displayable.cpp */
