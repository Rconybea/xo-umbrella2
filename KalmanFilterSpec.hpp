/* @file KalmanFilterSpec.hpp */

#pragma once

#include "filter/KalmanFilterStep.hpp"

namespace xo {
  namespace kalman {
    /* full specification for a kalman filter.
     *
     * For a textbook linear filter,  expect a KalmanFilterStep
     * instance to be independent of KalmanFilterState/KalmanFilterInput.
     * 
     * Relaxing this requirement for two reasons:
     * 1. (proper) want to allow filter with variable timing between observations,
     *    expecially if observations are event-driven.
     *    In that case it's likely that state transition matrices are a function
     *    of elapsed time between observations.  Providing filter state sk
     *    allows MkStepFn to use sk.tm()
     * 2. (sketchy) when observations represent market data,
     *    desirable to treat an observation as giving one-sided information
     *    about true value.   For example treat a bid price as evidence 
     *    true value is higher than that bid,  but don't want to constrain
     *    "how much higher".   Certainly no reason to think that
     *    bid price is normally distributed around fair value.
     *    Allow for hack in which we 
     *    and modulate error distribution "as if it were normal" to assess
     *    a non-gaussian error distribution
     */
    class KalmanFilterSpec {
    public:
      /* typical implementation will look something like:
       *    mk_step(KalmanFilterState const & sk,
       *            KalmanFilterInputPtr const & zkp1)
       *    {
       *      KalmanFilterTransition model = ...;
       *      KalmanFilterObservable obs = ...;
       *
       *      return KalmanFilterStep(sk, model, obs, zkp1);
       *    }
       */
      using MkStepFn = std::function<KalmanFilterStep
				     (ref::rp<KalmanFilterState> const & sk,
	                              KalmanFilterInputPtr const & zkp1)>;

    public:
      explicit KalmanFilterSpec(ref::rp<KalmanFilterStateExt> s0, MkStepFn mkstepfn)
	: start_ext_{std::move(s0)},
	  mk_step_fn_{std::move(mkstepfn)} {}

      ref::rp<KalmanFilterStateExt> const & start_ext() const { return start_ext_; }
      /* get step parameters (i.e. matrices F, Q, H, R)
       * for step t(k) -> t(k+1).
       *
       * We supply t(k) state s and t(k+1) observation z(k+1):
       * - to allow time stepping to be observation-driven
       * - to allow for selective outlier removal
       */
      KalmanFilterStep make_step(ref::rp<KalmanFilterState> const & sk,
				 ref::rp<KalmanFilterInput> const & zkp1) {
	return this->mk_step_fn_(sk, zkp1);
      } /*make_step*/

      void display(std::ostream & os) const;
      std::string display_string() const;

    private:
      /* starting state */
      ref::rp<KalmanFilterStateExt> start_ext_;

      /* creates kalman filter step object on demand;
       * step object specifies inputs to 1 step in discrete
       * linear kalman filter
       */
      MkStepFn mk_step_fn_;
    }; /*KalmanFilterSpec*/

    inline std::ostream &
    operator<<(std::ostream & os, KalmanFilterSpec const & x) {
      x.display(os);
      return os;
    } /*operator<<*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterSpec.hpp */
