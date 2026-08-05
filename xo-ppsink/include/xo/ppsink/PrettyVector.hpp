/** @file PrettyVector.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Lives in xo-ppsink, not xo-indentlog2, because it needs nothing from the
 *  arena-backed sink -- only pretty() and Prettifier<>.  Keeping it here means
 *  a subsystem that just wants to print a vector does not have to take a
 *  dependency on xo-indentlog2 (and through it xo-arena).
 **/

#pragma once

#include "pretty.hpp"
#include <vector>

namespace xo::pp {
    /** pretty-print a std::vector as
     *    [elt0, elt1, ..]
     *  Splits between elements, so a vector that doesn't fit breaks one
     *  element per line:
     *    [elt0,
     *      elt1,
     *      ..]
     *  Elements are emitted via pretty(), so element types without a
     *  Prettifier<> specialization render via operator<<.
     **/
    template <typename T>
    struct Prettifier<std::vector<T>> {
        static void print(PpSink & sink, const std::vector<T> & v) {
            sink.put("[").begin();

            bool first = true;
            for (const T & elt : v) {
                if (!first)
                    sink.put(",").split();
                first = false;

                pretty(sink, elt);
            }

            sink.end().put("]");
        }
    };
} /*namespace xo::pp*/

/* end PrettyVector.hpp */
