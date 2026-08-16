/* @file KalmanFilter.cpp */

#include "KalmanFilter.hpp"
#include "Eigen/src/Core/Matrix.h"
#include "KalmanFilterEngine.hpp"
#include "print_eigen.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <xo/ppsink/pp_time.hpp>      /* Prettifier<utc_nanos>: keeps xo's space-free format */

namespace xo {
    using xo::time::utc_nanos;
    //using logutil::matrix;
    using Eigen::MatrixXd;
    using Eigen::VectorXd;

    namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
        using xo::pp::scope;
        using xo::pp::tostr;
        using xo::pp::xtag;

        // ----- KalmanFilter -----

        KalmanFilter::KalmanFilter(KalmanFilterSpec spec)
            : filter_spec_{std::move(spec)},
              state_ext_{filter_spec_.start_ext()}
        {} /*ctor*/

        void
        KalmanFilter::notify_input(rp<KalmanFilterInput> const & input_kp1)
        {
            scope log(XO_ENTER0_(info));

            /* on entry:
             *    .state_ext refers to t(k)
             * on exit:
             *    .step refers to t(k+1)
             *    .state_ext refers to t(k+1)
             */

            log && log(xtag("step_dt",
                            input_kp1->tkp1() - this->state_ext_->tm()));

            /* establish step inputs for this filter step:
             *   F(k+1)   (system transition matrix)
             *   Q(k+1)   (system noise covariance matrix)
             *   H(k+1)   (observation coupling matrix)
             *   R(k+1)   (observation noise covariance matrix)
             *   z(k+1)   (observation vector)
             */
            this->step_ = this->filter_spec_.make_step(this->state_ext_, input_kp1);

            //if (lscope.enabled()) { lscope.log(xtag("step", this->step_)); }

            /* extrapolate filter state to t(k+1),
             * and correct based on z(k+1)
             */
            this->state_ext_ = KalmanFilterEngine::step(this->step_);

            //if (lscope.enabled()) { lscope.log(xtag("state_ext", this->state_ext_)); }
        } /*notify_input*/

        void
        KalmanFilter::display(std::ostream & os) const
        {
            os << "<KalmanFilter";
            os << xtag("filter_spec", filter_spec_);
            //os << xtag("step", step_);
            os << xtag("state_ext", state_ext_);
            os << ">";
        } /*display*/

        std::string
        KalmanFilter::display_string() const
        {
            return tostr(*this);
        } /*display_string*/
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilter.cpp */
