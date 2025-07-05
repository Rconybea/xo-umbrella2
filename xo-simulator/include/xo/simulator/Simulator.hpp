/* @file Simulator.hpp */

#pragma once

#include "xo/reactor/Reactor.hpp"
#include "SourceTimestamp.hpp"
#include "xo/reactor/ReactorSource.hpp"
#include "xo/refcnt/Refcounted.hpp"
//#include "time/Time.hpp"
#include <vector>

namespace xo {
    namespace sim {
        class TimeSlip;

        /* delay state-changing simulator command while handling
         * simulator events.  need this to permit reentrancy
         */
        struct ReentrantSimulatorCmd {
            enum SimulatorCmdEnum { NotifySourcePrimed, CompleteAddSource, CompleteRemoveSource };

            using ReactorSource = xo::reactor::ReactorSource;

        public:
            ReentrantSimulatorCmd() = default;
            ReentrantSimulatorCmd(SimulatorCmdEnum cmd,
                                  rp<reactor::ReactorSource> const & src)
                : cmd_{cmd}, src_{src} {}

            static ReentrantSimulatorCmd notify_source_primed(rp<ReactorSource> const & src) {
                return ReentrantSimulatorCmd(NotifySourcePrimed, src);
            }

            static ReentrantSimulatorCmd complete_add_source(rp<ReactorSource> const & src) {
                return ReentrantSimulatorCmd(CompleteAddSource, src);
            }

            static ReentrantSimulatorCmd complete_remove_source(rp<ReactorSource> const & src) {
                return ReentrantSimulatorCmd(CompleteRemoveSource, src);
            }

            SimulatorCmdEnum cmd() const { return cmd_; }
            rp<ReactorSource> const & src() const { return src_; }

        private:
            /* NotifySourcePrimed:    deferred Simulator.notify_source_primed(.src)
             * CompleteAddSource:     deferred Simulator.complete_add_source(.src)
             * CompleteRemoveSource:  deferred Simulator.complete_remove_source(.src)
             */
            SimulatorCmdEnum cmd_ = NotifySourcePrimed;
            /* if .cmd=NotifySourcePrimed|CompleteAddSource|CompleteRemoveSource:  reactor source */
            rp<ReactorSource> src_;
        }; /*ReentrantSimulatorCmd*/

        /* Generic simulator
         *
         * - time advances monotonically
         * - applies a modifiable set of sources
         *
         * A Simulator isn't an example of a Reactor,
         * because it can't work with arbitrary Sources
         * (may find it expedient to fake this later,
         * so we can easily adopt
         *    Source.notify_reactor_add() / Source.notify_reactor_remove())
         * in a simulation context
         */
        class Simulator : public reactor::Reactor {
        public:
            using ReactorSourcePtr = xo::reactor::ReactorSourcePtr;
            using ReactorSource = xo::reactor::ReactorSource;
            using utc_nanos = xo::time::utc_nanos;
            using nanos = xo::time::nanos;

        public:
            ~Simulator();

            static rp<Simulator> make(utc_nanos t0);

            /* value of .t0() is estabished in ctor.
             * it will not change except across call to .advance_one()
             * in particular .add_source() does not change .t0()
             */
            utc_nanos t0() const { return t0_; }

            /* timestamp of last event delivered */
            utc_nanos last_tm() const { return last_tm_; }
            /* total #of events delivered since sim start */
            uint64_t n_event() const { return n_event_; }

            /* true iff all simulation source are exhausted
             * a newly-created simulator is in the exhausted state;
             * it may transition to non-exhausted state across
             * call to .add_source()
             */
            bool is_exhausted() const { return this->src_v_.empty(); }

            /* true iff src has been added to this simulator
             * (by .add_source())
             */
            bool is_source_present(bp<ReactorSource> src) const;

            /* promise:
             *   .next_tm() > .t0() || .is_exhausted()
             *
             * .next_tm() may decrease across .add_source() call
             * .next_tm() may increase across .advance_one() call
             */
            utc_nanos next_tm() const;

            /* returns source that will be used for next simulator event.
             * nullptr if no remaining sources
             */
            ReactorSource * next_src() const;

            /* cross-reference realtime to simulated time,
             * for throttled replay
             */
            TimeSlip timeslip() const;

            /* compute throttled real time for next event.
             * caller supplies:
             * 1. a pair of timesstamps xref_ts = (sim_tm, real_tm)
             *    - xref_ts.sim_tm is time in simulation coords of last event
             *      (i.e. most recent available value of .last_tm())
             *    - xref_ts.real_tm is wall clock time associated with simtime
             * 2. a replay factor,  representing desired
             *      elapsed_simulation_time : elapsed_real_time
             *
             * return value is realtime delay to apply before next simulated event,
             * in order to maintain desired replay factor
             *
             * The incremental api here is intended to be used from a python thread.
             *
             * Expect python simulation loop like:
             *   import pysimulator
             *
             *   replay_factor = 1.0
             *   sim = pysimulator.Simulator.make(t0)
             *   sim.run_one()
             *   tslip = sim.timeslip()
             *   while(True):
             *     dt = sim.throttled_event_dt(tslip, replay_factor)
             *     sleep(dt)
             *     sim.run_one()
             *
             * This allows sleep() to be invoked from python,
             * which plays nicely with python threading model
             */
            nanos throttled_event_dt(TimeSlip xref_ts,
                                     double replay_factor) const;

            /* current contents of simulation heap,  in increasing time order.
             * copies heap to drain it in heap order
             */
            std::vector<SourceTimestamp> heap_contents() const;

            /* print heap contents to *p_scope.  intended for diagnostics */
            void log_heap_contents(xo::scope * p_scope) const;

            /* human-readable string identifying this simulator */
            std::string display_string() const;

            /* emit the first available event from a single simulation source.
             * resolve ties arbitrarily.
             *
             * returns the #of events dispatched
             * (expect this always = 1)
             */
            std::uint64_t advance_one_event();

            /* run simulation until earliest event time t satisfies t > t1
             */
            void run_until(utc_nanos t1);

            /* run simulation at realtime speed,  throttling according to replay_factor,
             * until either:
             * - simulation exhausted
             * - n events handled, if n>0
             * - sim clock reaches t1, if t1>t0
             *
             * see also .run_one(), .run_until(), .run_n(), .run()
             *
             * note: this method not suitable for use from python wrappers;
             *       would hold GIL until complete.
             *       for that use case better to implement throttled sim loop
             *       in python
             *
             * t1.  if > .t0,  limit sim to events with t < t1
             * n.   if > 0,  sim at most n events
             * replay_factor.  throttle sim to keep
             *                 {elapsed sim time} <= replay_factor * {elapsed real time}
             * return.  #of events simmed
             */
            uint64_t run_throttled_until(utc_nanos t1,
                                         int32_t n,
                                         double replay_factor);

            // ----- inherited from Reactor -----

            /* notification when nonprimed source becomes primed
             */
            virtual void notify_source_primed(bp<ReactorSource> src) override;

            /* add a new simulation source.
             * event that precede .t0 will be discarded.
             *
             * returns true if src added;  false if already present
             */
            virtual bool add_source(bp<ReactorSource> src) override;

            /* remove simulation source.
             * returns true if src removed;  false if was not present
             *
             * (not typically needed for simulations)
             */
            virtual bool remove_source(bp<ReactorSource> src) override;

            /* synonym for .advance_one_event() */
            virtual std::uint64_t run_one() override;

        private:
            explicit Simulator(utc_nanos t0);

            /* updates source timestamp in simulation heap.
             * preserves
             *
             * Require:
             * - src->is_primed()
             * - .sim_heap[.sim_heap.size - 1] already refers to src
             *
             * need_pop_flag,  if true, src is at back of heap vector,
             * need to pop before re-inserting.
             */
            void heap_update_source(ReactorSource * src,
                                    bool need_pop_flag);

            /* insert source into .sim_heap.
             * increase sim_heap.size() by +1
             */
            void heap_insert_source(ReactorSource * src);

            /* complete any reentrant work encountered
             * while deliverying another event
             */
            void complete_delivery_work();

            /* complete reentrant call to .add_source() */
            void complete_add_source(bp<ReactorSource> src);
            /* complete reentrant call to .remove_source() */
            void complete_remove_source(bp<ReactorSource> src);

            friend class RaiiDeliveryWork;

        private:
            /* simulation heap:
             * each unexhausted source appears
             * exactly once,  in increasing time order of next event
             *
             * Invariant:
             * - all sources s in .sim_heap satisfy:
             *   - s.is_exhausted() = false
             *   - s.t0() >= .t0
             */
            std::vector<SourceTimestamp> sim_heap_;

            /* initial simulation clock */
            utc_nanos t0_;

            /* time of most recent simulated event */
            utc_nanos last_tm_;

            /* #of simulated events handled */
            uint64_t n_event_ = 0;

            /* simulation sources
             * Invariant:
             * - all source s in .src_v satisfy:
             *   EITHER
             *     1.  s.is_exhausted() = true
             *   OR
             *     2.1 s.is_exhausted() = false
             *     2.2 s.t0() >= .t0
             */
            std::vector<ReactorSourcePtr> src_v_;

            /* reentrancy protection.  set during .advance_one_event() */
            bool delivery_in_progress_ = false;

            /* when certain Simulator methods are invoked
             * while in the midst of delivering another event,
             * must defer until delivery has completed
             */
            std::vector<ReentrantSimulatorCmd>  reentrant_cmd_v_;
        }; /*Simulator*/

    } /*namespace sim*/
} /*namespace xo*/

/* end Simulator.hpp */
