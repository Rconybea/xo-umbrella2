/* @file KalmanFilterStep.hpp */

#pragma once

#include <xo/ppsink/Prettifier.hpp>   /* pretty(PpSink&), Prettifier<> */
#include "KalmanFilterInput.hpp"
#include "KalmanFilterObservable.hpp"
#include "KalmanFilterState.hpp"
#include "KalmanFilterTransition.hpp"
#include <xo/timeutil/timeutil.hpp>

namespace xo {
    namespace kalman {
        /* encapsulate {state + observation} models for a single time step t(k).
         * Emitted by KalmanFilterSpec, q.v.
         */
        class KalmanFilterStepBase {
        public:
            KalmanFilterStepBase() = default;
            KalmanFilterStepBase(KalmanFilterTransition model,
                                 KalmanFilterObservable obs)
                : model_{std::move(model)},
                  obs_{std::move(obs)} {}

            /* aka system_model() */
            KalmanFilterTransition const & model() const { return model_; }
            KalmanFilterObservable const & obs() const { return obs_; }

        private:
            /* model for process being observed (state transition + noise) */
            KalmanFilterTransition model_;
            /* what can be observed (observables + noise) */
            KalmanFilterObservable obs_;
        }; /*KalmanFilterStepBase*/

        /* encapsulate {state + observation} models for a single time step t(k).
         * Emitted by KalmanFilterSpec, q.v.
         *
         * holds:
         *   x(k)
         *   P(k)
         *   F(k)
         *   H(k+1)
         *   z(k+1)
         *
         * contains all the inputs needed to compute:
         *   x(k+1)
         *   P(k+1)
         *
         * does not provide that result
         */
        class KalmanFilterStep : public KalmanFilterStepBase {
        public:
            using utc_nanos = xo::time::utc_nanos;
            using MatrixXd = Eigen::MatrixXd;
            using VectorXd = Eigen::VectorXd;

        public:
            KalmanFilterStep() = default;
            KalmanFilterStep(rp<KalmanFilterState> state,
                             KalmanFilterTransition model,
                             KalmanFilterObservable obs,
                             rp<KalmanFilterInput> zkp1)
                : KalmanFilterStepBase(model, obs),
                  state_{std::move(state)},
                  input_{std::move(zkp1)} {}

            rp<KalmanFilterState> const & state() const { return state_; }
            rp<KalmanFilterInput> const & input() const { return input_; }

            utc_nanos tkp1() const { return input_->tkp1(); }

            /* extrapolate kalman filter state forward to time
             * .tkp1() (i.e. to t(k+1));  computes
             *   x(k+1|k)
             *   P(k+1|k)
             * does not use the t(k+1) observations .input.z
             */
            rp<KalmanFilterState> extrapolate() const;

            /* compute kalman gain matrix K(k+1)
             * given extrapolated t(k+1) state skp1_ext = {x(k+1|k), P(k+1|k)}
             *
             * note that .state() != skp1_ext;  .state() reports {x(k), P(k)}
             */
            MatrixXd gain(rp<KalmanFilterState> const & skp1_ext) const;

            /* compute kalman gain vector K(k+1)
             * given extrapolated t(k+1) state skp1_ext = {x(k+1|k), P(k+1|k)},
             * on behalf of a single observation z[j].
             * actual observation z[j] is not given here,
             * just computing the gain vector.   i'th member of gain vector
             * gives effect of innovation on i'th member of kalman filter state.
             */
            VectorXd gain1(rp<KalmanFilterState> const & skp1_ext,
                           uint32_t j) const;

            /* compute correction to extrapolated filter state {x(k+1|k), P(k+1|k)},
             * for observation z(k+1) = .input.z()
             */
            rp<KalmanFilterStateExt> correct(rp<KalmanFilterState> const & skp1_ext);

            /* compute correction to extrapolated filter state skp1_ext = {x(k+1|k), P(k+1|k)},
             * for a single observation z(k+1, j) = .input.z()[j]
             */
            rp<KalmanFilterStateExt> correct1(rp<KalmanFilterState> const & skp1_ext,
                                                   uint32_t j);

            void pretty(xo::pp::PpSink & sink) const;
#ifdef OBSOLETE
            void display(std::ostream & os) const;
#endif
            std::string display_string() const;

        private:
            /* system state: timestamp, estimated process state, process covariance
             *               asof beginning of this step
             */
            rp<KalmanFilterState> state_;
            /* input: observations at time t(k+1) */
            KalmanFilterInputPtr input_;
        }; /*KalmanFilterStep*/

#ifdef OBSOLETE
        inline std::ostream &
        operator<<(std::ostream & os, KalmanFilterStep const & x) {
            x.display(os);
            return os;
        } /*operator<<*/
#endif

    } /*namespace kalman*/

    namespace pp {
        template <>
        struct Prettifier<xo::kalman::KalmanFilterStep> {
            static void print(PpSink & sink, const xo::kalman::KalmanFilterStep & x) {
                x.pretty(sink);
            }
        };
    }
} /*namespace xo*/

/* end KalmanFilterStep.hpp */
