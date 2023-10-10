/* @file AbstractSink.hpp */

#pragma once

#include "AbstractSource.hpp"
#include "xo/reflect/TaggedPtr.hpp"
#include "xo/reflect/TypeDescr.hpp"
//#include "time/Time.hpp"
#include "xo/indentlog/print/tag.hpp"
#include "xo/cxxutil/demangle.hpp"
#include <typeinfo>

namespace xo {
  namespace reactor {
    /* an event consumer.
     * note that event representation is not specified,
     * this helps avoid mandating a type hierarchy for events
     */
    class AbstractSink : public virtual AbstractEventProcessor {
    public:
      using TypeDescr = reflect::TypeDescr;
      using TaggedPtr = reflect::TaggedPtr;

    public:
      virtual ~AbstractSink() = default;

      /* if true: sources may produce events of any reflected type.
       *          sink will accept such events using .notify_ev_tp()
       *          for example see web_util/WebsocketSink
       *
       * if false (common): souce is expected to to produce events of
       *          a single type,  specified by .sink_ev_type()
       *          .notify_ev_tp() will downcast to that type.
       *          for example see reactor/Sink1
       *
       * polymorphic sinks pay for runtime polymorphism
       * (since WebsocketSink sends events in json format this is
       * expected to be negligible compared to message formatting)
       */
      virtual bool allow_polymorphic_source() const = 0;

      /* identify datatype for items expected by this sink */
      virtual TypeDescr sink_ev_type() const = 0;

      /* true iff this sink accepts volatile events.
       * volatile events are events that may be modified
       * or destroyed after being delivered to this sink.
       *
       * For example KalmanFilterSvc accepts volatile events,
       * but EventStore requires non-volatile events.
       */
      virtual bool allow_volatile_source() const = 0;

      /* counts lifetime #of incoming events for this sink */
      virtual uint32_t n_in_ev() const = 0;

      /* attach an input source.
       * typically this means calling src.add_callback()
       * with a function thats calls a .notify_xxx() method
       * on this Sink
       */
      virtual void attach_source(ref::rp<AbstractSource> const & src) = 0;

      /* accept incoming event,  given by tagged pointer */
      virtual void notify_ev_tp(TaggedPtr const & ev_tp) = 0;
    }; /*AbstractSink*/

  } /*namespace reactor*/
} /*namespace xo*/

/* end AbstractSink.hpp */
