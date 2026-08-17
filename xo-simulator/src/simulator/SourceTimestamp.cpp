/* file SourceTimestamp.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "SourceTimestamp.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/tag_ostream.hpp>
#include <xo/refcnt/Refcounted_pp.hpp>

namespace xo {
    namespace sim {
        void
        SourceTimestamp::display(std::ostream & os) const
        {
            /* xtag/tostr deliberately qualified rather than bound with a
             * using-declaration: the argument types (xo::time::utc_nanos,
             * xo::rp<>) live in namespace xo, so ADL reaches xo and would
             * pick up legacy xo::xtag in any TU that still sees indentlog.
             */
            os << "<SourceTimestamp";
            os << xo::pp::xtag("t0", t0_);
            os << xo::pp::xtag("src", rp<ReactorSource>(src_));
            os << ">";
        } /*display*/

        std::string
        SourceTimestamp::display_string() const
        {
            return xo::pp::tostr(*this);
        } /*display_string*/
    } /*namespace sim*/
} /*namespace xo*/

/* end SourceTimestamp.cpp */
