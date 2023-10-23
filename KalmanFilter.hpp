/* @file KalmanFilter.hpp */

#pragma once

#include "filter/KalmanFilterSpec.hpp"

namespace xo {
  namespace kalman {
    /* Specification for an ordinary discrete linear kalman filter.
     *
     * The filter generates estimates for a process observed at a discrete
     * set of times tk in {t0, t1, .., tn}
     *
     * At each time tk we have the following:
     *
     * 0. x(0) initial estimate at t(0)
     *    P(0) initial priors: error covariance matrix for x(0)
     *
     * 1. x_(k), [n x 1] vector:
     *    system state, denoted by vector.
     *    (state is not directly observable,
     *     filter will attempt to estimate it)
     *
     * 2. w_(k), [n x 1] vector
     *    Q(k),  [n x n] matrix
     *
     *    w_(k) denotes system noise,
     *    gaussian with covariance Q(k).
     *    noise w_(k) is not directly observable.
     *
     * 3. z(k), [m x 1] vector:
     *
     *    observation vector for time tk
     *
     * 4. v_(k), [m x 1] vector
     *    R(k), [m x m] matrix
     *
     *    v_(k) denotes observation errors,
     *    gaussian with covariance R(k).
     *    noise v_(k) is not directly observable.
     *
     * 5. F(k), [n x n] matrix
     *    state transition matrix
     *    model system evolves according to:
     *
     *    x_(k+1) = F(x).x_(k) + w_(k)
     *
     * 6. observations z(k) depend on system state:
     *
     *    z(k) = H(k).x_(k) + v_(k)    
     *     
     * 7. Kalman filter outputs:
     *    x(k), [n x 1] vector
     *    Q(k), [n x n] matrix
     *
     *    x(k) is optimal estimate for system state x_(k)
     *    P(k) is covariance matrix specifying confidence intervals
     *         for pairs (x(k)[i], x(k)[j])
     *
     * filter specification consists of:
     *    n, m, x(0), P(0), F(k), Q(k), H(k), R(k)
     * The cardinality of observations z(k) can vary over time,
     * so to be precise,  m can vary with tk;   write as m(k)
     *
     * More details:
     * - avoid having to specify t(k) in advance;
     *   instead defer until observation available
     *   so t(k) can be taken from polling timestamp
     */

    /* encapsulate a (linear) kalman filter
     * together with event publishing
     */
    class KalmanFilter {
    public:
      using MatrixXd = Eigen::MatrixXd;
      using VectorXd = Eigen::VectorXd;
      using utc_nanos = xo::time::utc_nanos;

    public:
      /* create filter with specification given by spec,  and initial state s0 */
      explicit KalmanFilter(KalmanFilterSpec spec);

      uint32_t step_no() const { return state_ext_->step_no(); }
      utc_nanos tm() const { return state_ext_->tm(); }
      KalmanFilterSpec const & filter_spec() const { return filter_spec_; }
      KalmanFilterStep const & step() const { return step_; }
      ref::rp<KalmanFilterStateExt> const & state_ext() const { return state_ext_; }

      /* notify kalman filter with input for time t(k+1) = input_kp1.tkp1()
       * Require: input.tkp1() >= .current_tm()
       * Promise:
       * - .tm() = input_kp1.tkp1()
       * - .step_no() = old .step_no() + 1
       * - .filter_spec_k, .step_k, .state_k updated
       *   for observations in input_kp1
       */
      void notify_input(ref::rp<KalmanFilterInput> const & input_kp1);

      void display(std::ostream & os) const;
      std::string display_string() const;

    private:
      /* specification for kalman filter;
       * produces process/observation matrices on demand
       */
      KalmanFilterSpec filter_spec_;

      /* filter step for most recent observation */
      KalmanFilterStep step_;

      /* filter state as of most recent observation;
       * result of applying KalmanFilterEngine::step() to contents of .step
       */
      ref::rp<KalmanFilterStateExt> state_ext_;
    }; /*KalmanFilter*/

    inline std::ostream &
    operator<<(std::ostream & os, KalmanFilter const & x) {
      x.display(os);
      return os;
    } /*operator<<*/

  } /*namespace kalman*/
} /*namespace xo*/


/* end KalmanFilter.hpp */
