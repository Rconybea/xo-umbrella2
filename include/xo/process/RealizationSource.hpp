/* @file RealizationSimSource.hpp */

#pragma once

#include "xo/reactor/ReactorSource.hpp"
#include "RealizationTracer.hpp"
#include "RealizationCallback.hpp"
#include "xo/callback/CallbackSet.hpp"
#include "xo/indentlog/scope.hpp"
#include <functional>

namespace xo {
    namespace process {
        /* use a discrete realization of a continuous stochastic process,
         * as a simulation source.
         *
         * 1. Realization is developed lazily,  (see RealizationTracer<T>)
         * 2. Use a fixed discretization interval to develop realization
         * 3. events are consumed by Sink
         *
         * Require:
         * - std::pair<utc_nanos, T> --convertible-to--> EventType
         * - EventSink.notify_source_exhausted()
         * - invoke EventSink(x), with x :: EventType
         */
        template <typename EventType, typename T, typename EventSink>
        class RealizationSourceBase : public xo::reactor::ReactorSource {
        public:
            using event_type = typename RealizationTracer<T>::event_type;
            using nanos = xo::time::nanos;

        public:
            ~RealizationSourceBase() {
                //constexpr char const * c_self = "RealizationSimSource<>::dtor";
                constexpr bool c_logging_enabled = false;

                scope log(XO_DEBUG(c_logging_enabled),
                          "delete instance",
                          xtag("p", this));
            } /*dtor*/

            static ref::rp<RealizationSourceBase>
            make(ref::rp<RealizationTracer<T>> const & tracer,
                 nanos ev_interval_dt,
                 EventSink const & ev_sink)
                {
                    using xo::scope;
                    using xo::xtag;

                    constexpr bool c_logging_enabled = false;

                    auto p = new RealizationSourceBase(tracer, ev_interval_dt, ev_sink);

                    scope log(XO_DEBUG(c_logging_enabled),
                              "create instance",
                              xtag("p", p),
                              xtag("bytes", sizeof(RealizationSourceBase)));

                    return p;
                } /*make*/

#ifdef NOT_IN_USE
            static ref::rp<RealizationSimSource> make(ref::rp<RealizationTracer<T>> tracer,
                                                      nanos ev_interval_dt,
                                                      EventSink && ev_sink)
                {
                    return new RealizationSimSource(tracer, ev_interval_dt, ev_sink);
                } /*make*/
#endif

            event_type const & current_ev() const { return this->tracer_->current_ev(); }
            nanos ev_interval_dt() const { return ev_interval_dt_; }

            /* supplying this to allow for setting up cyclic pointer references */
            EventSink * ev_sink_addr() { return &(this->ev_sink_); }

            /* deliver current event to sink */
            void sink_one() const {
                /* calling .ev_sink() can modify the callback set reentrantly
                 * (i.e. adding/removing callbacks)
                 * although this changes the state of .ev_sink,
                 * we want to treat this as not changing the state of *this
                 */
                RealizationSourceBase * self = const_cast<RealizationSourceBase *>(this);

                self->ev_sink_(this->tracer_->current_ev());
            } /*sink_one*/

            // ----- inherited from ReactorSource -----

            /* process realizations are always primed (at least for now) */
            virtual bool is_empty() const override { return false; }
            /* stochastic process api doesn't have an end time;
             * will need simulator to impose one
             */
            virtual bool is_exhausted() const override { return false; }

            virtual utc_nanos sim_current_tm() const override { return this->tracer_->current_tm(); }

            virtual std::string const & name() const override { return name_; }
            virtual void set_name(std::string const & x) override { this->name_ = x; }
            virtual bool debug_sim_flag() const override { return debug_sim_flag_; }
            virtual void set_debug_sim_flag(bool x) override { this->debug_sim_flag_ = x; }

            /* note:
             *   with replay_flag=true,  treats tm as lower bound
             */
            virtual std::uint64_t sim_advance_until(utc_nanos tm, bool replay_flag) override {
                std::uint64_t retval = 0ul;

                if(replay_flag) {
                    while(this->sim_current_tm() < tm) {
                        retval += this->deliver_one();
                    }
                } else {
                    this->tracer_->advance_until(tm);
                }

                return retval;
            } /*advance_until*/

            // ----- Inherited from AbstractSource -----

            virtual TypeDescr source_ev_type() const override {
                return reflect::Reflect::require<EventType>();
            }

            /* Tracer is intended always to deliver non-volatile events */
            virtual bool is_volatile() const override { return false; }

            virtual uint32_t n_out_ev() const override { return n_out_ev_; }
            /* no mechanism in RealizationSource to hold onto an outgoing event
             * see reactor::SecondarySource for contrary example
             */
            virtual uint32_t n_queued_out_ev() const override { return 0; }

            virtual std::uint64_t deliver_one() override {
                ++(this->n_out_ev_);
                this->sink_one();
                this->tracer_->advance_dt(this->ev_interval_dt_);

                return 1;
            } /*deliver_one*/

            virtual CallbackId attach_sink(ref::rp<reactor::AbstractSink> const & /*sink*/) override {
                /* see RealizationSource */
                assert(false);
                return CallbackId();
            }

            virtual void detach_sink(CallbackId /*id*/) override {
                /* see RealizationSource */
                assert(false);
            }

            virtual void display(std::ostream & os) const override {
                using xo::xtag;

                os << "<RealizationSourceBase"
                   << xtag("name", this->name())
                   << xtag("n_out_ev", this->n_out_ev())
                    //<< xtag("ev_interval_dt", ev_interval_dt_)
                   << ">";
            } /*display*/

            virtual void visit_direct_consumers(std::function<void (ref::brw<xo::reactor::AbstractEventProcessor>)> const &) override {
                assert(false);
            }

        protected:
            RealizationSourceBase(ref::rp<RealizationTracer<T>> const & tracer,
                                  nanos ev_interval_dt,
                                  EventSink const & ev_sink)
                : tracer_{tracer},
                  ev_sink_{std::move(ev_sink)},
                  ev_interval_dt_{ev_interval_dt} {}
            RealizationSourceBase(ref::rp<RealizationTracer<T>> const & tracer,
                                  nanos ev_interval_dt,
                                  EventSink && ev_sink)
                : tracer_{tracer},
                  ev_sink_{std::move(ev_sink)},
                  ev_interval_dt_(ev_interval_dt) {}

        private:
            static constexpr std::string_view sc_self_type = xo::reflect::type_name<RealizationSourceBase<EventType, T, EventSink>>();

        private:
            /* reporting name for this source -- use when .debug_sim_flag is set */
            std::string name_;
            /* if true reactor/simulator to log interaction with this source */
            bool debug_sim_flag_ = false;
            /* counts lifetime #of events */
            uint32_t n_out_ev_ = 0;
            /* produces events representing realized stochastic-process values */
            ref::rp<RealizationTracer<T>> tracer_;
            /* send stochastic-process events to this sink */
            EventSink ev_sink_;
            /* discretize process using this interval:
             * consecutive events from this simulation source will be at least
             * .ev_interval_dt apart
             */
            nanos ev_interval_dt_;
        }; /*RealizationSourceBase*/

        // ----- RealizationSource -----

        template<typename EventType, typename T>
        class RealizationSource
            : public RealizationSourceBase<EventType, T,
                                           xo::fn::NotifyCallbackSet<reactor::Sink1<EventType>,
                                                                     decltype(&reactor::Sink1<EventType>::notify_ev)>>

        {
        public:
            using TypeDescr = reflect::TypeDescr;
            using CallbackId = fn::CallbackId;
            using utc_nanos = xo::time::utc_nanos;
            using nanos = xo::time::nanos;

        public:
            static ref::rp<RealizationSource<EventType, T>> make(ref::rp<RealizationTracer<T>> const & tracer,
                                                                 nanos ev_interval_dt)
                {
                    return new RealizationSource<EventType, T>(tracer, ev_interval_dt);
                } /*make*/

            CallbackId add_callback(ref::rp<reactor::Sink1<EventType>> const & cb) {
                return this->ev_sink_addr()->add_callback(cb);
            } /*add_callback*/

            void remove_callback(CallbackId id) {
                this->ev_sink_addr()->remove_callback(id);
            } /*remove_callback*/

            // ----- inherited from AbstractSource -----

            /* alternative naming:
             *    .add_callback(sink)    <--> .attach_sink(sink)
             *    .remove_callback(sink) <--> .detach_sink(sink)
             */
            virtual CallbackId attach_sink(ref::rp<reactor::AbstractSink> const & sink) override {
                /* -------
                 * WARNING
                 * -------
                 * spent some time chasing down clang behavior here.
                 * the call to
                 *   reactor::Sink1<...>::require_native()
                 * fails unexpectedly because the template
                 *   Sink1<std::pair<utc_nanos,T>>
                 * and RealizationSource<T> may come from different modules.
                 */

                //using xo::scope;
                //using xo::xtag;

                /* checking that sink handles events of type T
                 * This is quick-n-dirty.   Want reflection here,   so we can write
                 * a runtime type test
                 *    sink->can_consume<T>()
                 * w/out exploding vtable size
                 */
                constexpr std::string_view c_self_name
                    = "RealizationSource::attach_sink";

                //scope lscope(c_self_name);
                //lscope.log(xtag("T", reflect::type_name<T>()));

                ref::rp<reactor::Sink1<EventType>> event_sink
                    = reactor::Sink1<EventType>::require_native(c_self_name, sink);

                return this->add_callback(event_sink);
            } /*attach_sink*/

            virtual void detach_sink(CallbackId id) override {
                /* see comment on .attach_sink() */

                this->remove_callback(id);
            } /*detach_sink*/

            virtual void display(std::ostream & os) const override {
                using xo::xtag;

                os << "<RealizationSource"
                   << xtag("name", this->name())
                   << xtag("n_out_ev", this->n_out_ev())
                    //<< xtag("ev_interval_dt", this->ev_interval_dt())
                   << ">";
            } /*display*/

            // ----- Inherited from AbstractEventProcessor -----

            virtual void visit_direct_consumers(std::function<void (ref::brw<xo::reactor::AbstractEventProcessor>)> const & fn) override {

                for(auto const & x : *(this->ev_sink_addr()))
                    fn(x.fn_.borrow());
            } /*visit_direct_consumers*/

        private:
            RealizationSource(ref::rp<RealizationTracer<T>> const & tracer,
                              nanos ev_interval_dt)
                : RealizationSourceBase
                  <EventType, T,
                   xo::fn::NotifyCallbackSet<reactor::Sink1<EventType>,
                                             decltype(&reactor::Sink1<EventType>::notify_ev)>
                   >(tracer,
                     ev_interval_dt,
                     fn::make_notify_cbset(&reactor::Sink1<EventType>::notify_ev))
                {}
        }; /*RealizationSource*/

    } /*namespace process*/
} /*namespace xo*/

/* end RealizationSource.hpp */
