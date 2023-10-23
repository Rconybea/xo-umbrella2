/* @file KalmanFilterEngine.hpp */

#pragma once

#include "filter/KalmanFilterStep.hpp"
#include "filter/KalmanFilterState.hpp"
#include "filter/KalmanFilterTransition.hpp"
#include "filter/KalmanFilterObservable.hpp"
#include "filter/KalmanFilterInput.hpp"

namespace xo {
  namespace kalman {
    class KalmanFilterEngine {
    public:
      using MatrixXd = Eigen::MatrixXd;
      using VectorXd = Eigen::VectorXd;
      using utc_nanos = xo::time::utc_nanos;

    public:
      /* evolution of system state + account for system noise,
       * before incorporating effect of observations z(k+1)
       *   x(k) --> x(k+1|k)
       *   P(k) --> P(k+1|k)
       *
       * tkp1.  time t(k+1) assoc'd with step k+1
       * sk.    filter state at time tk:
       *    sk.k  = k       step # (starts from 0)
       *    sk.tk = t(k)    time t(k) assoc'd with step #k
       *    sk.x = x(k)     estimated state at time tk
       *    sk.P = P(k)     quality of state estimate x(k)
       *                    (error covariance matrix)
       * Fk. state transition:
       *    Fk.F = F(k)     state transition matrix
       *    Fk.Q = Q(k)     covariance matrix for system noise
       *
       * returns propagated state estimate for t(k+1):
       *    retval.k = k+1
       *    retval.tk = t(k+1) = tkp1
       *    retval.x = x(k+1|k)
       *    retval.P = P(k+1|k)
       */
      static ref::rp<KalmanFilterState> extrapolate(utc_nanos tkp1,
						    ref::rp<KalmanFilterState> const & sk,
						    KalmanFilterTransition const & Fk);

      /* compute kalman gain matrix for filter step t(k) -> t(k+1)
       * Expensive implementation using matrix inversion
       *
       *                              T
       *   M(k+1) = H(k).P(k+1|k).H(k) + R(k)
       *
       *                         T       -1
       *   K(k+1) = P(k+1|k).H(k) .M(k+1)
       *
       * Require:
       * - skp1_ext.n_state() = Hkp1.n_state()
       *
       * skp1_ext.  extrapolated filter state at t(k+1)
       * Hkp1.      relates model state to observable variables,
       *            for step t(k) -> t(k+1)
       */
      static MatrixXd kalman_gain(ref::rp<KalmanFilterState> const & skp1_ext,
				  KalmanFilterObservable const & Hkp1);

      /* compute kalman gain for a single observation z(k)[j].
       * This is useful iff the observation error matrix R is diagonal.
       * For diagonal R we can present a set of observations z(k) serially
       * instead of all at once,  with lower time complexity
       *
       * Kalman Filter specifies some space with m observables.
       * j identifies one of those observables, indexing from 0.
       * This corresponds to row #j of H(k), and element R[j,j] of R.
       *
       * Effectively,  we are projecting the kalman filter assoc'd with
       * {skp1_ext, Hkp1} to a filter with a single observable variable z(k)[j],
       * then computing the (scalar) kalman gain for this 1-variable filter
       *
       * The gain vector tells us for each member of filter state,
       * how much to adjust our optimal estimate for that member for a unit
       * amount of innovation in observable #j,  i.e. for difference between
       * expected and actual value for that observable.
       */
      static VectorXd kalman_gain1(ref::rp<KalmanFilterState> const & skp1_ext,
				   KalmanFilterObservable const & Hkp1,
				   uint32_t j);

      /* correct extrapolated state+cov estimate;
       * also computes kalman gain
       *
       * Require:
       * - skp1_ext.n_state() = Hkp1.n_state()
       * - zkp1.n_obs() == Hkp1.n_observable()
       *
       * skp1_ext.      extrapolated kalman state + covaraince at t(k+1)
       * Hkp1.          relates model state to observable variables
       *                for step t(k) -> t(k+1)
       * zkp1.          observations for time t(k+1)
       *
       * return new filter state+cov
       */
      static ref::rp<KalmanFilterStateExt> correct(ref::rp<KalmanFilterState> const & skp1_ext,
						   KalmanFilterObservable const & Hkp1,
						   ref::rp<KalmanFilterInput> const & zkp1);

      /* correct extrapolated filter state for observation
       * of j'th filter observable z(k+1)[j]
       *
       * Can use this when observation errors are uncorrelated
       * (i.e. observation error matrix R is diagonal)
       */
      static ref::rp<KalmanFilterStateExt> correct1(ref::rp<KalmanFilterState> const & skp1_ext,
						    KalmanFilterObservable const & Hkp1,
						    ref::rp<KalmanFilterInput> const & zkp1,
						    uint32_t j);

      /* step filter from t(k) -> t(k+1)
       *
       * sk.        filter state from previous step:
       *            x (state vector), P (state covar matrix)
       * Fk.        transition-related params:
       *            F (transition matrix), Q (system noise covar matrix)
       * Hkp1.      observation-related params:
       *            H (coupling matrix), R (error covar matrix)
       * zkp1.      observations z(k+1) for time t(k+1)
       */
      static ref::rp<KalmanFilterStateExt> step(utc_nanos tkp1,
						ref::rp<KalmanFilterState> const & sk,
						KalmanFilterTransition const & Fk,
						KalmanFilterObservable const & Hkp1,
						ref::rp<KalmanFilterInput> const & zkp1);

      /* step filter from t(k) -> tk(k+1)
       * same as
       *   .step(tkp1, sk, step_spec.model(), step_spec.obs(), zkp1);
       *
       * step_spec.  encapsulates Fk (transition-related params)
       *             and Q (system noise covar matrix)
       */
      static ref::rp<KalmanFilterStateExt> step(KalmanFilterStep const & step_spec);

      /* step filter from t(k) -> t(k+1)
       *
       * sk.    filter state from previous step:
       *        x (state vector), P (state covar matrix)
       * Fk.    transition-related params:
       *        F (transition matrix), Q (system noise covar matrix)
       * Hkp1.  observation-related params:
       *        H (coupling matrix), R (error covar matrix)
       * zkp1.  observations z(k+1) for time t(k+1)
       * j.     identifies a single filter observable --
       *        step will only consume observation z(k+1)[j]
       */
      static ref::rp<KalmanFilterStateExt> step1(utc_nanos tkp1,
						 ref::rp<KalmanFilterState> const & sk,
						 KalmanFilterTransition const & Fk,
						 KalmanFilterObservable const & Hkp1,
						 ref::rp<KalmanFilterInput> const & zkp1,
						 uint32_t j);

      /* step filter from t(k) -> t(k+1)
       *
       * same as
       *   .step1(step_spec.tkp1(),
       *          step_spec.state(),
       *          step_spec.model(),
       *          step_spec.obs(),
       *          step_spec.input(),
       *          j);
       *
       * step_spec.  encapsulates
       *             x (state vector),
       *             P (state covar matrix),
       *             Fk (transition-related params),
       *             Q (system noise covar matrix)
       *             z (z(k+1), observations at time t(k+1))
       * j.          identifies a single filter observable --
       *             step will only consume observation z(k+1)[j]
       */
      static ref::rp<KalmanFilterStateExt> step1(KalmanFilterStep const & step_spec,
						 uint32_t j);
    }; /*KalmanFilterEngine*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterEngine.hpp */
