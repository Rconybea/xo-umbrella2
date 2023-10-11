/* @file AbstractSource.hpp */

#pragma once

#include "AbstractEventProcessor.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include "xo/callback/CallbackSet.hpp"
#include "xo/refcnt/Refcounted.hpp"
#include <string>

namespace xo {
    namespace web { class StreamEndpointDescr; }

    namespace reactor {
        class AbstractSink;

        template<typename T>
        class Sink1;

        /* abstract api for a source of events.
         * Event representation is left open:  Sources and Sinks
         * need to have compatible event representations,
         * and coordination is left to such (Source, Sink) pairs.
         *
         * See ReactorSource, for example
         *
         * Typically a Source will have one or more .add_callback()
         * methods, for listening to source events
         */
        class AbstractSource : public virtual AbstractEventProcessor {
        public:
            using StreamEndpointDescr = web::StreamEndpointDescr;
            using TypeDescr = reflect::TypeDescr;
            using CallbackId = fn::CallbackId;

        public:
            /* identify datatype for items delivered by this source */
            virtual TypeDescr source_ev_type() const = 0;

            /* if true:  event objects (see .source_ev_type())
             *           may be overwritten between callbacks.
             *           A sink that wants to capture events
             *           (e.g. EventStore<>) will need to deep-copy them
             * if false: event objects are preserved between callbacks.
             *
             * A source that stores events received from elsewhere (e.g. FifoQueue)
             * is probably volatile.
             *
             * A source that remembers (in explicit memory) every event it produces
             * is not volatile
             */
            virtual bool is_volatile() const = 0;

            /* counts #of outbound events ready for delivery,
             * but not yet sent */
            virtual uint32_t n_queued_out_ev() const = 0;
            /* counts lifetime #of events delivered.
             * see also AbstractSink.n_in_ev
             */
            virtual uint32_t n_out_ev() const = 0;

            /* if true,  simulator will report interaction with this source */
            virtual bool debug_sim_flag() const = 0;
            /* set .trace_sim_flag */
            virtual void set_debug_sim_flag(bool x) = 0;

            virtual CallbackId attach_sink(ref::rp<AbstractSink> const & sink) = 0;
            virtual void detach_sink(CallbackId id) = 0;

            /* endpoint for a websocket subscriber;
             * subscriber delivers events produced by this source
             */
            StreamEndpointDescr stream_endpoint_descr(std::string const & url_prefix);

            /* typically expect events to be delivered using a reactor or simulator.
             * (for example see reactor/Reactor, simulator/Simulator);
             * reactor allocates cpu, and controls event ordering across sources
             * when there are multiple sources.
             *
             * However, also possible for user code to invoke .deliver_one() directly.
             * Beware,  may get unpredictable results if attempt to do this on a source
             * that's also attached to a reactor.
             */
            virtual std::uint64_t deliver_one() = 0;

            /* convenience: call .deliver_one() n times,  return sum of results */
            std::uint64_t deliver_n(uint64_t n);

            /* convenience: call .deliver_one() until it returns 0
             * (beware of inexhaustible sources!)
             */
            std::uint64_t deliver_all();
        }; /*AbstractSource*/

        using AbstractSourcePtr = ref::rp<AbstractSource>;

    } /*namespace reactor*/
} /*namespace xo*/

/* end AbstractSource.hpp */
