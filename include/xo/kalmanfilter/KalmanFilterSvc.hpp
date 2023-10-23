/* @file KalmanFilterSvc.hpp */

#include "xo/reactor/Sink.hpp"
#include "xo/reactor/DirectSourcePtr.hpp"
#include "KalmanFilter.hpp"
#include "KalmanFilterInputSource.hpp"
#include "KalmanFilterOutputCallback.hpp"
#include "xo/callback/CallbackSet.hpp"

namespace xo {
    namespace kalman {
        /* encapsulate a passive KalmanFilter
         * instance as an active event consumer+producer
         *
         * sinks that want to consume KalmanFilterSvc events will use
         * .attach_sink() (or .add_callback())
         */
        class KalmanFilterSvc : public xo::reactor::Sink1<ref::rp<KalmanFilterInput>>,
                                public xo::reactor::DirectSourcePtr<ref::rp<KalmanFilterStateExt>> {
        public:
            using AbstractSource = xo::reactor::AbstractSource;

        public:
            /* named ctor idiom */
            static ref::rp<KalmanFilterSvc> make(KalmanFilterSpec spec);

            KalmanFilter const & filter() const { return filter_; }

            /* notify incoming observations;  will trigger kalman filter step */
            void notify_ev(ref::rp<KalmanFilterInput> const & input_kp1) override;

            // ----- inherited from reactor::AbstractSink -----

            /* filter captures KF input pointer */
            virtual bool allow_volatile_source() const override { return false; }
            virtual uint32_t n_in_ev() const override { return n_in_ev_; }
            virtual void display(std::ostream & os) const override;

            // ----- inherited from reactor::AbstractSource -----

            /* note: correct since KalmanFilterEngine.extrapolate()
             *       always creates new state object
             */
            virtual bool is_volatile() const override { return false; }

            // ----- Inherited from AbstractEventProcessor -----

        private:
            KalmanFilterSvc(KalmanFilterSpec spec);

        private:
            /* passive kalman filter */
            KalmanFilter filter_;
            /* receive filter input from this source; see .attach_input() */
            ref::rp<KalmanFilterInputSource> input_src_;
            /* counts lifetime #of input events (see .notify_ev()) */
            uint32_t n_in_ev_ = 0;
            /* publish filter state updates to these callbacks */
            fn::RpCallbackSet<KalmanFilterOutputCallback> pub_;
        }; /*KalmanFilterSvc*/
    } /*namespace kalman*/
} /*namespace xo*/

/* KalmanFilterSvc.hpp */
