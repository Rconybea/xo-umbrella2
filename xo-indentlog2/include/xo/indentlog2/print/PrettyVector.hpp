/** @file PrettyVector.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "Pretty.hpp"
#include <vector>

namespace xo::print {
    /** pretty-print a std::vector as
     *    [elt0, elt1, ..]
     *  Splits between elements, so a vector that doesn't fit breaks one
     *  element per line:
     *    [elt0,
     *      elt1,
     *      ..]
     *  Elements are emitted via pp_write(), so element types without a
     *  Pretty<> specialization render via operator<<.
     **/
    template <typename T>
    struct Pretty<std::vector<T>> {
        static void print(PpSink & sink, const std::vector<T> & v) {
            sink.put("[").begin();

            bool first = true;
            for (const T & elt : v) {
                if (!first)
                    sink.put(",").split();
                first = false;

                pp_write(sink, elt);
            }

            sink.end().put("]");
        }
    };
} /*namespace xo::print*/

/* end PrettyVector.hpp */
