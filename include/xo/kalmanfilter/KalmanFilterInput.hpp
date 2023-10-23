/* @file KalmanFilterInput.hpp */

#pragma once

#include "xo/reflect/SelfTagging.hpp"
//#include "time/Time.hpp"
//#include "xo/refcnt/Refcounted.hpp"
#include <Eigen/Dense>
#include <cstdint>

namespace xo {
    /* FIXME.  hack here to get member access working for reflection */
    namespace vf { class StrikesetKfinput; }

    namespace kalman {
        /* represents a single kalman filter input event
         * comprising:
         *   - time tkp1
         *   - observation vector z[]
         *   - presence bits presence[] for z
         *   - observation errors Rd[]
         */
        class KalmanFilterInput : public reflect::SelfTagging {
        public:
            using TaggedRcptr = xo::reflect::TaggedRcptr;
            using utc_nanos = xo::time::utc_nanos;
            using VectorXb = Eigen::Array<bool, Eigen::Dynamic, 1>;
            using VectorXi = Eigen::VectorXi;
            using VectorXd = Eigen::VectorXd;
            using uint32_t = std::uint32_t;

        public:
            KalmanFilterInput() = default;

            static ref::rp<KalmanFilterInput> make(utc_nanos tkp1,
                                                   VectorXb const & presence,
                                                   VectorXd const & z,
                                                   VectorXd const & Rd);

            /* create input,  with all presence bits set + not using Rd */
            static ref::rp<KalmanFilterInput> make_present(utc_nanos tkp1,
                                                           VectorXd const & z);

            /* reflect KalmanFilterInput object representation */
            static void reflect_self();

            /* alt name -- concession to reactor::DirectSource
             *             when event type is KalmanFilterInput
             */
            utc_nanos tm() const { return tkp1_; }

            utc_nanos tkp1() const { return tkp1_; }
            uint32_t n_obs() const { return z_.size(); }
            VectorXb const & presence() const { return presence_; }
            VectorXd const & z() const { return z_; }
            VectorXd const & Rd() const { return Rd_; }

            /* computes reindexer keep[]:
             *   .presence[keep[j*]]
             * is the j*'th true value in .presence
             */
            VectorXi make_kept_index() const;

            virtual void display(std::ostream & os) const;
            std::string display_string() const;

            // ----- inherited from SelfTagging -----

            virtual TaggedRcptr self_tp() override;

        protected:
            KalmanFilterInput(utc_nanos tkp1, VectorXb presence, VectorXd z, VectorXd Rd)
                : tkp1_(tkp1),
                  presence_{std::move(presence)},
                  z_{std::move(z)},
                  Rd_{std::move(Rd)} {}

            friend class xo::vf::StrikesetKfinput;

        private:
            /* t(k+1) - asof time for observations .z */
            utc_nanos tkp1_ = xo::time::timeutil::epoch();
            /* [m x 1] presence vector.
             *         an observation z[j] is present iff .presence[j] is true.
             *         rows/columns for an absent observation are removed from filter matrices
             */
            VectorXb presence_;
            /* [m x 1] observation vector z(k) */
            VectorXd z_;

            /* [m x 1] observation error vector Rd(k).
             *         This represents a side channel for passing the diagonal of
             *         observation matrix R(k), when both:
             *         (a) error of different observations are assumed to be uncorrelated (likely)
             *         (b) error variance is derived from input data, e.g. because of
             *             some input preprocessing.
             *
             *         It's up to KalmanFilterSpec::MkStepFn to opt-in to using this information,
             *         which requires agreement with any input preparation step.
             *
             *         This variable could just as well provide observation error matrix R
             *
             * NOTE: perhaps-cleaner alternative would be to inherit KalmanFilterInput to
             *       introduce additional state,  then MkStepFn can dynamic_cast
             */
            VectorXd Rd_;
        }; /*KalmanFilterInput*/

        using KalmanFilterInputPtr = ref::rp<KalmanFilterInput>;

        inline std::ostream &
        operator<<(std::ostream & os, KalmanFilterInput const & x)
        {
            x.display(os);
            return os;
        } /*operator<<*/

    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInput.hpp */
