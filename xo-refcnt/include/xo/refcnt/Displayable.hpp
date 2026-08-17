/* @file Displayable.hpp */

#pragma once

#include "Refcounted.hpp"
#include <xo/ppsink/PpSink.hpp>
#include <xo/ppsink/pretty.hpp>

#include <concepts>

namespace xo {
    namespace ref {
        class Displayable : public Refcount {
        public:
            using PpSink = xo::pp::PpSink;

        public:
            /* write some kind of human-readable representation on stream */
            virtual void pretty(PpSink & pp) const = 0;
            // implement display_string() in derived classes that also have xo-indentlog2
            virtual std::string display_string() const = 0;
        }; /*Displayable*/

    } /*namespace ref*/
} /*namespace xo*/

namespace xo::pp {
    /** Prettifier for anything deriving from ref::Displayable.
     *  Must live here to insure that it's consistently applied
     *  (else ODR violation!)
     **/
    template <typename T>
        requires std::derived_from<T, xo::ref::Displayable>
    struct Prettifier<T> {
        static void print(PpSink & sink, const T & x) {
            x.pretty(sink);
        }
    };
} /*namespace xo::pp*/

/* end Displayable.hpp */
