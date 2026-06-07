/* @file KalmanFilterState.hpp */

#pragma once

#include "xo/reflect/SelfTagging.hpp"
#include "KalmanFilterInput.hpp"
#include "KalmanFilterTransition.hpp"
//#include "time/Time.hpp"
#include <Eigen/Dense>
#include <functional>
#include <cstdint>

namespace xo {
    namespace kalman {
        /* encapsulate state (i.e. initial state,  and output after each step)
         * for a kalman filter
         */
        class KalmanFilterState : public reflect::SelfTagging {
        public:
            using TaggedRcptr = reflect::TaggedRcptr;
            using utc_nanos = xo::time::utc_nanos;
            using VectorXd = Eigen::VectorXd;
            using MatrixXd = Eigen::MatrixXd;
            using uint32_t = std::uint32_t;

        public:
            static rp<KalmanFilterState> make();
            static rp<KalmanFilterState> make(uint32_t k,
                                                   utc_nanos tk,
                                                   VectorXd x,
                                                   MatrixXd P,
                                                   KalmanFilterTransition transition);
            virtual ~KalmanFilterState() = default;

            /* reflect KalmanFilterState object representation */
            static void reflect_self();

            uint32_t step_no() const { return k_; }
            utc_nanos tm() const { return tk_; }
            /* with n = .n_state():
             *   x_ is [n x 1] vector
             *   P_ is [n x n] matrix,
             */
            uint32_t n_state() const { return x_.size(); }
            VectorXd const & state_v() const { return x_; }
            MatrixXd const & state_cov() const { return P_; }

            KalmanFilterTransition const & transition() const { return transition_; }

            virtual void display(std::ostream & os) const;
            std::string display_string() const;

            // ----- inherited from SelfTagging -----

            virtual TaggedRcptr self_tp() override;

        private:
            KalmanFilterState();
            KalmanFilterState(uint32_t k,
                              utc_nanos tk,
                              VectorXd x,
                              MatrixXd P,
                              KalmanFilterTransition transition);

            friend class KalmanFilterStateExt;

        private:
            /* step# k,  advances by +1 on each filter step */
            uint32_t k_ = 0;
            /* time t(k) */
            utc_nanos tk_;
            /* [n x 1] (estimated) system state xk = x(k) */
            VectorXd x_;
            /* [n x n] covariance matrix for error assoc'd with with x(k)
             *   P(i,j) is the covariance of (ek[i], ek[j]),
             * where ex(k) is the difference (x(k) - x_(k))
             * between estimated state x(k)
             * (= this->x_) and model state x_(k)
             */
            MatrixXd P_;

            /* F, Q matrices driving .x, .P */
            KalmanFilterTransition transition_;
        }; /*KalmanFilterState*/

        inline std::ostream & operator<<(std::ostream & os,
                                         KalmanFilterState const & s)
        {
            s.display(os);
            return os;
        } /*operator<<*/

        /* KalmanFilterStateExt:
         * adds additional details from filter step to KalmanFilterState
         */
        class KalmanFilterStateExt : public KalmanFilterState {
        public:
            using TaggedRcptr = reflect::TaggedRcptr;
            using MatrixXd = Eigen::MatrixXd;
            using int32_t = std::int32_t;

        public:
            static rp<KalmanFilterStateExt> make();
            static rp<KalmanFilterStateExt> make(uint32_t k,
                                                      utc_nanos tk,
                                                      VectorXd x,
                                                      MatrixXd P,
                                                      KalmanFilterTransition transition,
                                                      MatrixXd K,
                                                      int32_t j,
                                                      rp<KalmanFilterInput> zk);

            /* create state object for initial filter state */
            static rp<KalmanFilterStateExt> initial(utc_nanos t0,
                                                         VectorXd x0,
                                                         MatrixXd P0);

            /* reflect KalmanFilterStateExt object representation */
            static void reflect_self();

            int32_t observable() const { return j_; }
            MatrixXd const & gain() const { return K_; }
            rp<KalmanFilterInput> const & zk() const { return zk_; }

            virtual void display(std::ostream & os) const override;

            // ----- inherited from SelfTagging -----

            virtual TaggedRcptr self_tp() override;

        private:
            KalmanFilterStateExt() = default;
            KalmanFilterStateExt(uint32_t k,
                                 utc_nanos tk,
                                 VectorXd x,
                                 MatrixXd P,
                                 KalmanFilterTransition transition,
                                 MatrixXd K,
                                 int32_t j,
                                 rp<KalmanFilterInput> zk);

        private:
            /* if -1:  not used;
             * if >= 0: identifies j'th of m observables;
             * gain .K applies just to information obtainable from
             * observing that scalar variable
             */
            int32_t j_ = -1;
            /* if .j is -1:
             *   [n x n] kalman gain
             * if .j >= 0:
             *   [n x 1] kalman gain for observable #j
             */
            MatrixXd K_;
            /* input leading to state k.
             * empty for initial state (i.e. when .k is 0)
             */
            rp<KalmanFilterInput> zk_;
        }; /*KalamnFilterStateExt*/
    } /*namespace filter*/
} /*namespace xo*/

/* end KalmanFilterState.hpp */
