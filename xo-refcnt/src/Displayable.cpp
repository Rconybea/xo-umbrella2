/* @file Displayable.cpp */

#include "Displayable.hpp"
#include <xo/ppsink/pretty_ostream.hpp>

namespace xo {
    using xo::pp::tostr0;

    namespace ref {
        std::string
        Displayable::display_string() const {
            return tostr0(*this);
        } /*display_string*/
    } /*namespace ref*/
} /*namespace xo*/

/* end Displayable.cpp */
