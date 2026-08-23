/* @file AbstractEventProcessor.hpp */

#pragma once

#include <xo/refcnt/Refcounted.hpp>
#include <functional>
#include <string>
#include <vector>
#include <xo/ppsink/Prettifier.hpp>   /* pretty(PpSink&), Prettifier<> */
#include <concepts>   /* std::derived_from */

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

            /** render self into @p sink **/
            virtual void pretty(xo::pp::PpSink & sink) const = 0;
            /** human-readable string identifying this source **/
            virtual std::string display_string() const;
        }; /*AbstractEventProcessor*/

    } /*namespace reactor*/

    namespace pp {
        /** Anything in the AbstractEventProcessor hierarchy renders through
         *  its virtual pretty().
         *
         *  CONSTRAINED PARTIAL specialization, not one exact specialization per
         *  class: an rp<T> renders via Prettifier<intrusive_ptr<T>>, which
         *  needs Prettifier<T> for the STATIC T -- and that T is routinely an
         *  intermediate abstract class (ReactorSource, AbstractSink, Sink1<U>)
         *  rather than the concrete sink.  Specializing per class would mean
         *  chasing each intermediate as it turned up.
         **/
        template <typename T>
            requires std::derived_from<T, xo::reactor::AbstractEventProcessor>
        struct Prettifier<T> {
            static void print(PpSink & sink, const T & x) {
                x.pretty(sink);
            }
        };
    }
} /*namespace xo*/

/* end AbstractEventProcessor.hpp */
