/* @file AbstractEventProcessor.hpp */

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include <functional>
#include <vector>
#include <string>

namespace xo {
    namespace reactor {
        /* common base class for {AbstractSource, AbstractSink}.
         * An event processor can be:
         * 1. an event source (inherits AbstractSource)
         * 2. an event sink (inherits AbstractSink)
         * 3. both source+sink (inherits both)
         */
        class AbstractEventProcessor : virtual public ref::Refcount {
        public:
            /* reporting name for this source.  ideally unique,  but not required */
            virtual std::string const & name() const = 0;
            /* set .name */
            virtual void set_name(std::string const & x) = 0;

            /* find all event processors ep reachable from x (i.e. downstream from x).
             * report each such ep exactly once
             */
            static std::vector<rp<AbstractEventProcessor>> map_network(rp<AbstractEventProcessor> const & x);

            /* visit direct downstream consumers c[i] of this event processor.
             * call ep(c[i]) for each such consumer.
             */
            virtual void visit_direct_consumers(std::function<void (bp<AbstractEventProcessor> ep)> const & fn) = 0;

            /* write representation to stream */
            virtual void display(std::ostream & os) const = 0;
            /* human-readable string identifying this source */
            virtual std::string display_string() const;
        }; /*AbstractEventProcessor*/

        inline std::ostream &
        operator<<(std::ostream & os, AbstractEventProcessor const & src) {
            src.display(os);
            return os;
        } /*operator<<*/

    } /*namespace reactor*/
} /*namespace xo*/

/* end AbstractEventProcessor.hpp */
