/** @file cmpresult.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "cmpresult.hpp"
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/tag_ostream.hpp>
#include <iostream>

namespace xo {
    using xo::pp::PpSink;
    using xo::pp::field;
    using xo::pp::xtag;

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

#ifdef OBS
    namespace pp {
        template <>
        void
        Prettifier<xo::mm::comparison>::print(PpSink & sink, const xo::mm::comparison & x)
        {
            sink.put(comparison2str(x));
        }
    }
#endif
} /*namespace xo*/

/* end cmpresult.cpp */
