/** @file pretty_array.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  Prettifier<std::array<T, N>> -- the fixed-size counterpart to
 *  PrettyVector.hpp, and the ppsink replacement for legacy xo-indentlog
 *  print/array.hpp (the ostream inserter) together with the
 *  ppdetail<std::array<T,N>> in print/pretty_vector.hpp.
 *
 *  Renders identically to a vector:
 *    [elt0, elt1, ..]
 *  splitting between elements, so an array that does not fit breaks one
 *  element per line.
 *
 *  Separate header rather than an addition to PrettyVector.hpp so that a TU
 *  pays for <array> or <vector> but not both.  Deliberately NOT a generic
 *  "any sized range" specialization: std::string and std::string_view are
 *  ranges too, and they must keep rendering as strings.
 **/

#pragma once

#include "pretty.hpp"
#include <array>
#include <cstddef>

namespace xo::pp {
    template <typename T, std::size_t N>
    struct Prettifier<std::array<T, N>> {
        static void print(PpSink & sink, const std::array<T, N> & v) {
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

/* end pretty_array.hpp */
