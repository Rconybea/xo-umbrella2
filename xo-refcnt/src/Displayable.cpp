/* @file Displayable.cpp */

#include "Displayable.hpp"

namespace xo {
    using xo::pp::tostr0;

    namespace ref {
#ifdef OBSOLETE
        std::string
        Displayable::display_string() const {
            return tostr0(*this);
        } /*display_string*/
#endif
    } /*namespace ref*/
} /*namespace xo*/

/* end Displayable.cpp */
