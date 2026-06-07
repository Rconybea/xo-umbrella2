/* @file KalmanFilterStep.cpp */

#include "KalmanFilterStep.hpp"
#include "KalmanFilterEngine.hpp"
#include "KalmanFilterState.hpp"
#include "xo/indentlog/scope.hpp"

namespace xo {
    using xo::scope;
    using xo::tostr;
    using xo::xtag;
    using Eigen::MatrixXd;
    using Eigen::VectorXd;

    namespace kalman {
        rp<KalmanFilterState>
        KalmanFilterStep::extrapolate() const
        {
            return KalmanFilterEngine::extrapolate(this->tkp1(),
                                                   this->state(),
                                                   this->model() /*transition*/);
        } /*extrapolate*/

        MatrixXd
        KalmanFilterStep::gain(rp<KalmanFilterState> const & skp1_ext) const
        {
            return KalmanFilterEngine::kalman_gain(skp1_ext,
                                                   this->obs());
        } /*gain*/

        VectorXd
        KalmanFilterStep::gain1(rp<KalmanFilterState> const & skp1_ext,
                                uint32_t j) const
        {
            return KalmanFilterEngine::kalman_gain1(skp1_ext,
                                                    this->obs(),
                                                    j);

        } /*gain1*/

        rp<KalmanFilterStateExt>
        KalmanFilterStep::correct(rp<KalmanFilterState> const & skp1_ext)
        {
            return KalmanFilterEngine::correct(skp1_ext,
                                               this->obs(),
                                               this->input());
        } /*correct*/

        rp<KalmanFilterStateExt>
        KalmanFilterStep::correct1(rp<KalmanFilterState> const & skp1_ext,
                                   uint32_t j)
        {
            return KalmanFilterEngine::correct1(skp1_ext,
                                                this->obs(),
                                                this->input(),
                                                j);
        } /*correct1*/

        void
        KalmanFilterStep::display(std::ostream & os) const
        {
            //scope lscope("KalmanFilterStep::display");

            os << "<KalmanFilterStep";
            //lscope.log("state:");
            os << xtag("state", state_);
            //lscope.log("model:");
            os << xtag("model", this->model());
            //lscope.log("obs:");
            os << xtag("obs", this->obs());
            //lscope.log("input:");
            os << xtag("input", this->input());
            os << ">";
        } /*display*/

        std::string
        KalmanFilterStep::display_string() const
        {
            return tostr(*this);
        } /*display_string*/
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterStep.cpp */
