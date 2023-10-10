/* @file LastReducer.hpp */

#pragma once

#include "reactor/Reducer.hpp"
#include <array>

namespace xo {
  namespace reactor {
    /* implementation record used in LastReducer.
     * LastReducer (see below) remembers a single event,
     * + will be updated on successive calls to
     * LastReducer.include_event()
     *
     * need to remember the _first_ (& therefore earliest)
     * event timestamp in such a wave,  since that establishes when simulator
     * should deliver the event -- even if event is subsequently
     * overwritten.
     *
     * once event is delivered,  timestamp can reset
     *
     * otherwise if upstream producer sends events with
     * future timestamps,  can get indefinite postponement
     * with simulation clock failing to catch up to event time.
     * 
     */
   
    template<typename Event>
    class EventRecd {
    public:
      using utc_nanos = xo::time::utc_nanos;

    public:
      EventRecd() = default;
      EventRecd(utc_nanos tm, Event ev) : trigger_tm_{tm}, ev_{ev} {}
      EventRecd(utc_nanos tm, Event && ev) : trigger_tm_{tm}, ev_{std::move(ev)} {}

    public:
      /* if sim, deliver event when simulation clock reaches
       * .trigger_tm;  .trigger_tm can be earlier than .ev time
       */
      utc_nanos trigger_tm_;
      /* event to deliver */
      Event ev_;
    };

    /* reducer that just remembers the last event
     *
     * Require:
     * - Event is null-contructible
     * - Event is copyable
     *
     * LastReducer provides reentrancy support.  This support doesn't operate
     * if Event copy is not deep,  e.g. for Event = rp<Foo>n
     *
     *              .include_event()
     *   /-------\  -----------------> /------\
     *   | empty |                     | full |
     *   \-------/ <-----------------  \------/
     *      .         .annex_one()        .
     *      .                             .
     *   .is_empty()=true              .is_empty()=false
     */
    template<typename Event, typename EventTimeFn = StructEventTimeFn<Event>>
    class LastReducer : public ReducerBase<Event, EventTimeFn> {
    public:
      using utc_nanos = xo::time::utc_nanos;

    public:
      LastReducer() = default;
      LastReducer(EventTimeFn const & evtfn) : ReducerBase<Event, EventTimeFn>(evtfn) {}

      bool is_empty() const { return empty_flag_; }
      /* require: .is_empty() = false */
      utc_nanos next_tm() const {
	return this->last_ev_[this->last_ix_].trigger_tm_;
	//return this->event_tm(this->last_ev_[this->last_ix_]);
      }
      /* #of events stored in this reducer (0 or 1) */
      uint32_t n_event() const { return this->empty_flag_ ? 0 : 1; }

      Event const & last_annexed_ev() const {
	return this->last_ev_[1 - this->last_ix_].ev_;
      }

      EventRecd<Event> & include_event_aux(Event const & ev) {
	EventRecd<Event> & evr
	  = this->last_ev_[this->last_ix_];

	if (this->empty_flag_) {
	  /* evr.trigger_tm will be preserved across
	   * successive calls to .include_event();
	   * until .annex_one()
	   */
	  evr.trigger_tm_ = this->event_tm(ev);

	  this->empty_flag_ = false;
	}

	return evr;
      } /*include_event_aux*/

      void include_event(Event const & ev) {
	EventRecd<Event> & evr
	  = this->include_event_aux(ev);

	evr.ev_ = ev;
      } /*include_event*/

      void include_event(Event && ev) {
	EventRecd<Event> & evr
	  = this->include_event_aux(ev);
	
	evr.ev_ = std::move(ev);
      } /*include_event*/

      Event & annex_one() {
	std::uint32_t annexed_ix = this->last_ix_;

	/* since .empty_flag is true,
	 * next call to .include_event_aux() will
	 * capture new timestamp
	 */
	this->empty_flag_ = true;
	this->last_ix_ = (1 - this->last_ix_);

	return this->last_ev_[annexed_ix].ev_;
      } /*annex_one*/

      // ----- Inherited from ReducerBase -----

      //utc_nanos event_tm(Event const & ev) const { return this->event_tm_fn_(ev); }

    private:
      /* true when reducer contains 0 queued events,
       * not counting any annexed event
       */
      bool empty_flag_ = true;

      /* .last_ev[.last_ix] updated by .include_event()
       */
      std::uint32_t last_ix_ = 0;
      /* remember two events
       * (a) a single queued event  (updated by .include_event())
       * (b) a single removed event (reported by .annex_one())
       *
       * roles of .last_ev[0], .last_ev[1] reverse each time .annex_one() runs
       */
      std::array<EventRecd<Event>, 2> last_ev_;
    }; /*LastReducer*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end LastReducer.hpp */
