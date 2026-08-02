/** @file flatstring_pretty.hpp
 *
 *  Author: Roland Conybeare, Jul 2025
 *
 *  Transitional compatibility header: the legacy xo-indentlog ppdetail<>
 *  specialization for flatstring<N>.  This is the ONLY remaining xo-indentlog
 *  dependency in xo-flatstring; nothing else in the subsystem (headers, tests,
 *  cmake) references it.
 *
 *  ppsink needs no counterpart: flatstring<N> is convertible to
 *  std::string_view, so xo::pp::pretty() already renders it through its
 *  string-like leaf path with no Prettifier<> specialization at all.
 *
 *  Retire this once legacy consumers stop pretty-printing flatstring through
 *  xo-indentlog.  Note the specialization only makes flatstring print as an
 *  *atomic* (ppdetail_atomic); dropping it changes legacy pretty-printer
 *  layout rather than breaking compilation, so removal wants an output
 *  comparison, not just a green build.
 **/

#pragma once

#include "flatstring.hpp"
#include "flatstring_iostream.hpp"
#include <xo/indentlog/print/ppdetail_atomic.hpp>

namespace xo {
#ifndef ppdetail_atomic
    namespace print {
        struct ppindentinfo;

        template <std::size_t N>
        struct ppdetail<flatstring<N>> {
            static bool print_pretty(const ppindentinfo & ppii,
                                     const flatstring<N> & x) {
                return ppdetail_atomic<flatstring<N>>::print_pretty(ppii, x);
            };
        };
    }
#endif
}

/** end flatstring_pretty.hpp **/
