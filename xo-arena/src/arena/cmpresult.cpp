/** @file cmpresult.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "cmpresult.hpp"
#include <xo/ppsink/pretty_struct.hpp>

namespace xo {
    using xo::pp::PpSink;
    using xo::pp::field;

    namespace mm {
        const char *
        comparison2str(comparison x)
        {
            switch (x) {
            case comparison::invalid:
                break;
            case comparison::comparable:
                return "cmp";
            case comparison::incomparable:
                return "!cmp";
            }

            return "?comparison";
        }

        void
        cmpresult::pretty(PpSink & sink) const
        {
            sink.pretty_struct("cmpresult",
                               field("err", err_),
                               field("cmp", cmp_));
        }

    } /*namespace mm*/

} /*namespace xo*/

/* end cmpresult.cpp */
