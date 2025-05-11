/* @file RealizationTracer.hpp */

#pragma once

#include "StochasticProcess.hpp"
#include "xo/refcnt/Refcounted.hpp"

namespace xo {
    namespace process {
        //template<typename T, typename EventSink> class RealizationSimSource;

        /* One-way iteration over a realization (i.e. sampled path)
         * belonging to a stochastic process.
         * has a monotonically increasing 'current time'.
         * can be adapted as a simulation source
         *
         * Example:
         *   utc_nanos t0 = ...;
         *   double sdev = 1.0;
         *   Seed<xoshiro256ss> seed;
         *   auto process = LogNormalProcess::make(t0, sdev, seed);
         *   auto tracer = RealizationTracer<double>::make(process.get());
         */
        template <typename T>
        class RealizationTracer : public ref::Refcount {
        public:
            using Process = xo::process::StochasticProcess<T>;
            using process_type = Process;
            /* something like std::pair<utc_nanos, T> */
            using event_type = typename Process::event_type;
            using utc_nanos = xo::time::utc_nanos;
            using nanos = xo::time::nanos;

        public:
            static rp<RealizationTracer> make(rp<Process> const & p) {
                return new RealizationTracer(p);
            }

            event_type const & current_ev() const { return current_; }
            utc_nanos current_tm() const { return current_.first; }
            /* value of this path at time t */
            T const & current_value() const { return current_.second; }
            rp<Process> const & process() const { return process_; }

            /* sample with fixed time:
             * - advance to time t+dt,  where t=.current_tm()
             * - return new time and process value
             *
             * can use .advance_dt(dt) to avoid copying T
             */
            std::pair<utc_nanos, T> next_dt(nanos dt) {
                this->advance_dt(dt);

                return this->current_;
            } /*next_dt*/

            std::pair<utc_nanos, T> next_eps(double eps) {
                this->advance_eps(eps);

                return this->current_;
            } /*next_eps*/

            /* sample with fixed time:
             * - advance to point t+dt,  with dt specified.
             */
            void advance_dt(nanos dt) {
                utc_nanos t1 = this->current_.first + dt;

                this->advance_until(t1);
            } /*advance_dt*/

            void advance_until(utc_nanos t1) {
                event_type ev0 = this->current_;

                if(t1 <= ev0.first) {
                    /* tracer state already past t1 */
                } else {
                    T x1 = this->process_->exterior_sample(t1, ev0);

                    /* careful! may not alter .current until after call to exterior_sample()
                     * returns
                     */

                    this->current_.first = t1;
                    this->current_.second = x1;
                }
            } /*advance_until*/

#ifdef NOT_IN_USE // need StochasticProcess.hitting_time() for this
            /* sample with max change in process value eps.
             * requires that T defines a norm under which eps
             * can be interpreted
             */
            virtual void advance_eps(double eps) = 0;
#endif

        private:
            RealizationTracer(rp<Process> const & p)
                : current_(event_type(p->t0(), p->t0_value())), process_(p) {}

        private:
            /* current (time, processvalue) associated with this realization */
            event_type current_;

            /* develop a sampled realization of this stochastic process */
            rp<Process> process_;
        }; /*RealizationTracer*/

    } /*namespace process*/
} /*namespace xo*/

/* end RealizationTracer.hpp */
