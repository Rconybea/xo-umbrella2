/* file SourceTimestamp.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "SourceTimestamp.hpp"
#include "xo/indentlog/print/tag.hpp"
#include "xo/indentlog/print/tostr.hpp"

namespace xo {
    using xo::xtag;
    using xo::tostr;

    namespace sim {
        void
        SourceTimestamp::display(std::ostream & os) const
        {
            os << "<SourceTimestamp";
            os << xtag("t0", t0_);
            os << xtag("src", ref::rp<ReactorSource>(src_));
            os << ">";
        } /*display*/

        std::string
        SourceTimestamp::display_string() const
        {
            return tostr(*this);
        } /*display_string*/
    } /*namespace sim*/
} /*namespace xo*/

/* end SourceTimestamp.cpp */
