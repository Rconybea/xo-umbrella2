/** @file ratio_pp.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  ppsink-native structured printing for xo::ratio::ratio: Prettifier<ratio<Int>>.
 *
 *  Opt-in.  Include ratio.hpp alone for the type with no printing at all,
 *  ratio_iostream.hpp for the ostream inserter, this header to render a ratio
 *  into a PpSink -- e.g. sink.pp(r), or xo::pp::xtag("r", r).
 *
 *  NB xo-ratio does NOT declare a dependency on xo-ppsink: the library itself
 *  needs nothing from it, and xo_ratio is a header-only target consumed by
 *  xo-unit and its examples, which should not acquire a link edge for a
 *  printer they do not use.  A subsystem including THIS header must declare
 *  its own xo_ppsink dependency.  (Same arrangement as
 *  xo-reflect/TypeDescr_ppdetail.hpp.)  xo-ratio's own utest does declare it,
 *  so this header is compiled and tested rather than merely shipped.
 *
 *  Renders exactly what operator<< does -- @c <ratio 2/3> -- and that is
 *  deliberate, not incidental.  If the two disagreed, a ratio would print
 *  differently depending on whether some TU happened to include this header:
 *  with it, xtag("r", r) routes through Prettifier; without it, through the
 *  operator<< fallback.  Keeping them byte-identical removes that class of
 *  surprise rather than documenting it.  Keep them in step.
 *
 *  The win over just letting the fallback handle it: this renders via
 *  std::to_chars and put(), so a ppsink-only TU printing a ratio never
 *  instantiates the operator<< path and never needs <ostream>.
 **/

#pragma once

#include "ratio.hpp"
#include <xo/ppsink/Prettifier.hpp>
#include <charconv>
#include <cstring>
#include <limits>
#include <string_view>

namespace xo::pp {
    /** @brief pretty-print a ratio into a PpSink, as @c <ratio num/den>
     *
     *  Constrained to integral component types because std::to_chars is.
     *  ratio's numeric_concept is broader than that (it admits e.g.
     *  boost::rational, std::complex, xo::unit::quantity), so for those the
     *  primary Prettifier template stays empty and pretty() falls through to
     *  operator<< from ratio_iostream.hpp -- which is what happened for every
     *  component type before this header existed.
     **/
    template <std::integral Int>
    struct Prettifier<xo::ratio::ratio<Int>> {
        static void print(PpSink & sink, const xo::ratio::ratio<Int> & x) {
            /* digits10 excludes the leading digit of the widest value and the
             * sign; +3 covers both with slack, so to_chars cannot fail here.
             */
            constexpr std::size_t c_int_z = std::numeric_limits<Int>::digits10 + 3;

            constexpr std::string_view c_prefix = "<ratio ";

            /* "<ratio " + num + "/" + den + ">" */
            char buf[c_prefix.size() + c_int_z + 1 + c_int_z + 1];

            char * p = buf;
            char * const e = buf + sizeof(buf);

            std::memcpy(p, c_prefix.data(), c_prefix.size());
            p += c_prefix.size();

            p = std::to_chars(p, e, x.num()).ptr;
            *p++ = '/';
            p = std::to_chars(p, e, x.den()).ptr;
            *p++ = '>';

            /* one put(): a ratio is a leaf, with no break opportunity inside */
            sink.put(std::string_view(buf, static_cast<std::size_t>(p - buf)));
        }
    };
} /*namespace xo::pp*/

/* end ratio_pp.hpp */
