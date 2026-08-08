/** @file pretty_pair.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  Prettifier<std::pair<T, U>> -- the ppsink replacement for legacy xo-indentlog
 *  print/pair.hpp.
 *
 *  Renders as
 *    [first, second]
 *  splitting between the two members, so a pair that does not fit breaks one
 *  member per line.  Shape matches PrettyVector.hpp / pretty_array.hpp.
 *
 *  NB legacy print/pair.hpp declared its operator<< **inside namespace std**,
 *  which is undefined behaviour (only explicit specializations of std templates
 *  may be added there) -- it worked because ADL finds it for std::pair.  A
 *  Prettifier<> specialization needs no such trick: it lives in xo::pp and is
 *  found by the Prettifier machinery rather than by ADL.
 *
 *  Also note the separator: legacy rendered "[a b]", this renders "[a, b]",
 *  matching PrettyVector.
 **/

#pragma once

#include "pretty.hpp"
#include <utility>

namespace xo::pp {
    template <typename T, typename U>
    struct Prettifier<std::pair<T, U>> {
        static void print(PpSink & sink, const std::pair<T, U> & x) {
            sink.put("[").begin();

            pretty(sink, x.first);

            sink.put(",").split();

            pretty(sink, x.second);

            sink.end().put("]");
        }
    };
} /*namespace xo::pp*/

/* end pretty_pair.hpp */
