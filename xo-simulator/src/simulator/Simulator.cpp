/* @file Simulator.cpp */

//#include "time/Time.hpp" /*need this 1st for tag(., time_point)*/
#include "init_simulator.hpp"
#include "Simulator.hpp"
#include "TimeSlip.hpp"
#include "xo/indentlog/scope.hpp"
#include <thread>
#include <algorithm>
#include <string_view>

namespace xo {
    using xo::reactor::ReactorSource;
    using xo::time::utc_nanos;
    using xo::time::nanos;

    namespace sim {
        class RaiiDeliveryWork {
        public:
            RaiiDeliveryWork(Simulator * sim) : sim_{sim} {
                this->sim_->delivery_in_progress_ = true;
            }

            ~RaiiDeliveryWork() {
                this->sim_->delivery_in_progress_ = false;
                this->sim_->complete_delivery_work();
            }

            Simulator * sim_ = nullptr;
        }; /*RaiiDeliveryWork*/

        rp<Simulator>
        Simulator::make(utc_nanos t0) {
            return new Simulator(t0);
        } /*make*/

        Simulator::Simulator(utc_nanos t0) : t0_(t0)
        {
            XO_SUBSYSTEM_REQUIRE(simulator);
        } /*ctor*/

        Simulator::~Simulator() {
            scope log(XO_ENTER0(verbose), "clear heap..");

            this->sim_heap_.clear();

            if (log.enabled()) {
                log("visit .src_v", xtag("size", this->src_v_.size()));
                for (size_t i=0; i<this->src_v_.size(); ++i) {
                    log(":src_v[", i, "] ", this->src_v_[i].get());
                }
            }

            log && log("clear .src_v", xtag("size", this->src_v_.size()));

            this->src_v_.clear();
        } /*dtor*/

        bool
        Simulator::is_source_present(bp<ReactorSource> src) const
        {
            for (ReactorSourcePtr const & s : this->src_v_) {
                if (s == src)
                    return true;
            }

            return false;
        } /*is_source_pesent*/

        utc_nanos
        Simulator::next_tm() const {
            if(this->sim_heap_.empty()) {
                /* 0 remaining events in simulator */
                return this->t0();
            }

            return this->sim_heap_.front().t0();
        } /*next_tm*/

        ReactorSource*
        Simulator::next_src() const {
            if (this->sim_heap_.empty()) {
                /* 0 remaining events in simulator */
                return nullptr;
            }

            return this->sim_heap_.front().src();
        } /*next_src*/

        void
        Simulator::notify_source_primed(bp<ReactorSource> src)
        {
            scope log(XO_ENTER1(always, src->debug_sim_flag()));

            bp<ReactorSource> sim_src
                = bp<ReactorSource>::from(src);

            log && log(xtag("src", (sim_src.get() != nullptr)),
                       xtag("src.name", src->name()));

            if(!sim_src)
                return;

            log && log(xtag("sim.name", sim_src->name()),
                       xtag("src.current_tm", sim_src->sim_current_tm()),
                       xtag("sim_heap.size", this->sim_heap_.size()));

            if (this->delivery_in_progress_) {
                log && log("reentrant call to .notify_source_primed(), defer",
                           xtag("src.name", src->name()));

                /* defer reentrant work until delivery completes
                 * see .complete_delivery_work()
                 */
                this->reentrant_cmd_v_.push_back
                    (ReentrantSimulatorCmd::notify_source_primed(src.promote()));
            } else {
                /* inform Simulator when a source transitions from
                 * 'notready' to 'ready'.
                 *
                 * this means:
                 * - source knows its next event
                 * - source should be put back into .sim_heap
                 */
                this->heap_insert_source(sim_src.get());
            }

            //if (lscope.enabled())
            //  this->log_heap_contents(&lscope);
        } /*notify_source_primed*/

        void
        Simulator::complete_add_source(bp<ReactorSource> src)
        {
            /* also add to simulation heap */
            this->sim_heap_.push_back(SourceTimestamp(src->sim_current_tm(),
                                                      src.get()));

            /* use std::greater<> because we need a min-heap;
             * smallest timestamp at the front
             */
            std::push_heap(this->sim_heap_.begin(),
                           this->sim_heap_.end(),
                           std::greater<SourceTimestamp>());
        } /*complete_add_source*/

        bool
        Simulator::add_source(bp<ReactorSource> sim_src)
        {
            scope log(XO_ENTER1(always, sim_src->debug_sim_flag()));

            log && log("enter",
                       xtag("src", sim_src.get()),
                       xtag("src.name", sim_src->name()));

            if(!sim_src || this->is_source_present(sim_src))
                return false;

            log && log("advance to t0",
                       xtag("t0", this->t0()));

            sim_src->sim_advance_until(this->t0(), false /*!replay_flag*/);

            this->src_v_.push_back(sim_src.promote());

            if(sim_src->is_exhausted()) {
                log && log("source exhausted!");
            } else {
                sim_src->notify_reactor_add(this /*reactor*/);

                log && log(xtag("src.sim_current_tm", sim_src->sim_current_tm()));

                if (sim_src->is_empty()) {
                    log && log("empty source, do not insert into .sim_heap");

                    /* if source is empty,  don't add to sim heap yet.
                     * when source becomes non-empty,  source will invoke
                     *   .notify_source_primed()
                     * which will insert it into .sim_heap[]
                     */
                    ;
                } else if (this->delivery_in_progress_) {
                    log && log("reentrant add non-empty source, delay");

                    /* defer reentrant work until delivery completes
                     * see .complete_delivery_work()
                     */
                    this->reentrant_cmd_v_.push_back
                        (ReentrantSimulatorCmd::complete_add_source(sim_src.promote()));
                } else {
                    log && log("non-empty source, add to .sim_heap");

                    this->complete_add_source(sim_src);
                }
            }

            return true;
        } /*add_source*/

        void
        Simulator::complete_remove_source(brw<ReactorSource> sim_src)
        {
            /* rebuild .sim_heap,  with sim_src removed */
            std::vector<SourceTimestamp> sim_heap2;

            for(SourceTimestamp const & item : this->sim_heap_) {
                if(item.src() == sim_src.get()) {
                    /* item refers to the source we are removing -> discard */
                    ;
                } else {
                    sim_heap2.push_back(item);

                    std::push_heap(sim_heap2.begin(),
                                   sim_heap2.end(),
                                   std::greater<SourceTimestamp>());
                }

                /* now discard .sim_heap,  replacing with sim_heap2 */
                this->sim_heap_ = std::move(sim_heap2);
            }
        } /*complete_remove_source*/

        bool
        Simulator::remove_source(brw<ReactorSource> sim_src)
        {
            scope log(XO_DEBUG(sim_src->debug_sim_flag()));

            log && log("enter",
                       xtag("src", sim_src.get()),
                       xtag("src.name", sim_src->name()));

            //brw<ReactorSource> sim_src = brw<ReactorSource>::from(src);

            if(!sim_src || !this->is_source_present(sim_src))
                return false;

            /* WARNING: O(n)implementation here */

            if (this->delivery_in_progress_) {
                /* defer reentrant work until delivery completes.
                 * see .complete_delivery_work()
                 */
                this->reentrant_cmd_v_.push_back
                    (ReentrantSimulatorCmd::complete_remove_source(sim_src.promote()));
            } else {
                this->complete_remove_source(sim_src);
            }

            return true;
        } /*remove_source*/

        std::uint64_t
        Simulator::run_one() {
            return this->advance_one_event();
        } /*run_one*/

        void
        Simulator::heap_update_source(ReactorSource * src, bool need_pop_flag)
        {
            /* Require:
             *   .sim_heap[.sim_heap.size - 1] already refers to src
             * just updating timestamp here
             */

            std::size_t simheap_z
                = this->sim_heap_.size();

            scope log(XO_DEBUG(src->debug_sim_flag()),
                      xtag("src.name", src->name()),
                      xtag("simheap_z", simheap_z),
                      xtag("src.sim_current_tm", src->sim_current_tm()));

            if (need_pop_flag)
                this->sim_heap_.pop_back();
            /* re-insert at new timestamp */
            this->sim_heap_.push_back(SourceTimestamp(src->sim_current_tm(), src));

            /* use std::greater<> because we need a min-heap;
             * smallest timestamp at the front
             */
            std::push_heap(this->sim_heap_.begin(),
                           this->sim_heap_.end(),
                           std::greater<SourceTimestamp>());
        } /*heap_update_source*/

        void
        Simulator::heap_insert_source(ReactorSource * src)
        {
            /* santify check -- src should not currently appear in heap */
            for (SourceTimestamp const & src_recd : this->sim_heap_) {
                if(src_recd.src() == src) {
                    /* uh oh.   src is already present in heap! */
                    assert(false);
                }
            }

            // don't need this: .heap_update_source() will insert
            //this->sim_heap_.push_back(SourceTimestamp(src->sim_current_tm(), src));

            this->heap_update_source(src, false /*!need_pop_flag*/);
        } /*heap_insert_source*/

        void
        Simulator::complete_delivery_work()
        {
            for (ReentrantSimulatorCmd const & cmd : this->reentrant_cmd_v_) {
                scope log(XO_DEBUG(cmd.src() && cmd.src()->debug_sim_flag()),
                          "complete reentrant work",
                          xtag("src.name", cmd.src()->name()));

                switch (cmd.cmd()) {
                case ReentrantSimulatorCmd::NotifySourcePrimed:
                    this->notify_source_primed(cmd.src());
                    break;
                case ReentrantSimulatorCmd::CompleteAddSource:
                    this->complete_add_source(cmd.src());
                    break;
                case ReentrantSimulatorCmd::CompleteRemoveSource:
                    this->complete_remove_source(cmd.src());
                    break;
                }

                //if (lscope.enabled())
                //  this->log_heap_contents(&lscope);
            }

            this->reentrant_cmd_v_.clear();
        } /*complete_delivery_work*/

        TimeSlip
        Simulator::timeslip() const
        {
            utc_nanos real_tm = std::chrono::system_clock::now();
            utc_nanos  sim_tm = this->next_tm();

            return TimeSlip(sim_tm, real_tm);
        } /*timeslip*/

        nanos
        Simulator::throttled_event_dt(TimeSlip xref,
                                      double replay_factor) const
        {
            if (replay_factor <= 0.0)
                replay_factor = 1e-6;

            /* hi_sim_tm: simtime for next event to be handled */
            utc_nanos hi_sim_tm  = this->next_tm();
            /* desired elapsed /real time/ from start of simulation to
             * to when simulation handles event @ hi_sim_tm
             */
            nanos sim_dt = (hi_sim_tm - xref.sim_tm());
            auto hi_real_tm = (xref.real_tm()
                               + std::chrono::duration_cast<nanos>(sim_dt / replay_factor));
            utc_nanos now_tm = std::chrono::system_clock::now();

            if (now_tm < hi_real_tm)
                return hi_real_tm - now_tm;
            else
                return nanos(0);
        } /*next_throttled_tm*/

        std::vector<SourceTimestamp>
        Simulator::heap_contents() const
        {
            std::vector<SourceTimestamp> heap = this->sim_heap_;
            std::vector<SourceTimestamp> retval;

            while (!heap.empty()) {
                retval.push_back(heap.front());

                std::pop_heap(heap.begin(), heap.end(),
                              std::greater<SourceTimestamp>());
                heap.pop_back();
            }

            return retval;
        } /*heap_contents*/

        void
        Simulator::log_heap_contents(scope * p_scope) const
        {
            std::vector<SourceTimestamp> heap = this->sim_heap_;

            p_scope->log("/ sim heap contents:");
            p_scope->log("| t0 name n_in_ev n_queued_out_ev n_out_ev");

            while(!heap.empty()) {
                SourceTimestamp const & ts = heap.front();

                p_scope->log("|"
                             , " ", ts.t0()
                             , " ", ts.src()->name()
                             , " ", ts.src()->n_queued_out_ev()
                             , " ", ts.src()->n_out_ev());

                std::pop_heap(heap.begin(), heap.end(),
                              std::greater<SourceTimestamp>());
                heap.pop_back();
            }

            p_scope->log("\\");
        } /*print_heap_contents*/

        std::string
        Simulator::display_string() const
        {
            return "<Simulator>";
        } /*display_string*/

        std::uint64_t
        Simulator::advance_one_event()
        {
            bool debug_flag = (this->loglevel() <= log_level::chatty);

            if(this->sim_heap_.empty()) {
                scope log(XO_DEBUG(debug_flag));

                /* nothing todo */
                return 0;
            }

            uint32_t old_heap_z = this->sim_heap_.size();

            /* *src is source with earliest timestamp */
            ReactorSource * src
                = this->sim_heap_.front().src();

            utc_nanos src_tm = this->sim_heap_.front().t0();

            scope log(XO_DEBUG(debug_flag),
                      xtag("threshold-loglevel", this->loglevel()),
                      xtag("src", src != nullptr),
                      xtag("src.name", src->name()),
                      xtag("sim.src_tm", src_tm),
                      xtag("src.sim_current_tm", src->sim_current_tm()),
                      xtag("heap_z", old_heap_z));

            /* NOTE: src.current_tm() isn't preserved across
             *       call to src.deliver_one()
             */
            uint64_t retval = 0;

            {
                RaiiDeliveryWork raii_work(this);

                retval = src->deliver_one();

                this->last_tm_ = src_tm;
                this->n_event_ += retval;

                /* note that src.t0 may have advanced */

                /* moves just-consumed timestamp event for src
                 * to back of .sim_heap
                 */
                std::pop_heap(this->sim_heap_.begin(),
                              this->sim_heap_.end(),
                              std::greater<SourceTimestamp>());

                /* now .sim_heap[.sim_heap.size() = 1].src() is src,
                 * with stale timestamp
                 */

                if(src->is_exhausted() || src->is_notprimed()) {
                    /* remove src from .sim_
                     * - if src->is_exhausted(),  permanently
                     * - if src->is_notready(),  until source calls
                     *   .notify_source_ready()
                     */
                    this->sim_heap_.pop_back();
                } else {
                    this->heap_update_source(src, true /*need_pop_flag*/);
                }

                assert(raii_work.sim_);
            }

            return retval;
        } /*advance_one_event*/

        void
        Simulator::run_until(utc_nanos t1)
        {
            assert(!this->delivery_in_progress_);

            while(!this->is_exhausted()) {
                utc_nanos t = this->next_tm();

                if(t > t1)
                    break;

                this->advance_one_event();
            } /*loop until done*/
        } /*run_until*/

        uint64_t
        Simulator::run_throttled_until(utc_nanos t1,
                                       int32_t n_max,
                                       double replay_factor)
        {
            Subsystem::verify_all_initialized();

            scope log(XO_ENTER0(info));

            assert(!this->delivery_in_progress_);

            uint64_t n = 0;

            if(!this->is_exhausted()) {
                n += this->run_one();
            }

            /* cross-reference real time with sim time */
            TimeSlip tslip = this->timeslip();

            while(!this->is_exhausted()) {
                if ((n_max > 0) && (n >= static_cast<uint64_t>(n_max))) {
                    /* reached limit on #of events simmed */
                    return n;
                }

                if ((t1 > this->t0()) && (this->next_tm() > t1)) {
                    /* reached limit on sim time */
                    return n;
                }

                /* if sim time passing faster than realtime (scaled by replay_factor),
                 * wait for real elapsed time to catch up
                 */
                nanos wait_dt = this->throttled_event_dt(tslip, replay_factor);

                if (wait_dt > std::chrono::milliseconds(1)) {
                    log && log(xtag("sleep_dt", wait_dt));

                    std::this_thread::sleep_for(wait_dt);
                } else {
                    /* don't bother throttling for period less than 1ms,  linux != rtos */
                }

                n += this->run_one();
            }

            return n;
        } /*run_throttled_until*/

    } /*namespace sim*/
} /*namespace xo*/

/* end Simulator.cpp */
