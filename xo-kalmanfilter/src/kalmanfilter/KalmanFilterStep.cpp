/* @file KalmanFilterStep.cpp */

#include "KalmanFilterStep.hpp"
#include "KalmanFilterEngine.hpp"
#include "KalmanFilterState.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */

namespace xo {
    using Eigen::MatrixXd;
    using Eigen::VectorXd;

    namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
        using xo::pp::scope;
        using xo::pp::tostr0;
        using xo::pp::xtag;

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
            return tostr0(*this);
        } /*display_string*/
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterStep.cpp */
