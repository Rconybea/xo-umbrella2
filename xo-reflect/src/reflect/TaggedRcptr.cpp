/* file TaggedRcptr.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "TaggedRcptr.hpp"
#include <xo/indentlog2/print/tostr.hpp>

namespace xo {
    using xo::pp::xtag;
    using xo::pp::tostr;
    using xo::pp::field;

    namespace reflect {
        void
        TaggedRcptr::pretty(PpSink & sink) const
        {
            sink.pretty_struct("TaggedRcptr",
                               field("type", this->td()->canonical_name()),
                               field("addr", (void*)(this->rc_address())));
        }

        std::string
        TaggedRcptr::display_string() const {
            return tostr(*this);
        } /*display_string*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end TaggedRcptr.cpp */
