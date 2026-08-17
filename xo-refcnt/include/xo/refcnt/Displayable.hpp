/* @file Displayable.hpp */

#pragma once

#include "Refcounted.hpp"
#include <xo/ppsink/PpSink.hpp>

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

/* end Displayable.hpp */
