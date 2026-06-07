/* @file KalmanFilterObservable.hpp */

#pragma once

//#include "time/Time.hpp"
#include <Eigen/Dense>
#include <cstdint>

namespace xo {
    namespace kalman {
        class KalmanFilterObservable {
        public:
            using MatrixXd = Eigen::MatrixXd;
            using VectorXi = Eigen::VectorXi;

        public:
            KalmanFilterObservable() = default;

            /* keep maps back to canonical observations z(j).
             * H, R have been re-indexed
             *
             * If all m observations are included,  then keep will be:
             *   [0, .., m-1]
             */
            KalmanFilterObservable(VectorXi keep, MatrixXd H, MatrixXd R)
                : keep_{std::move(keep)}, H_{std::move(H)}, R_{std::move(R)} {
                assert(this->check_ok());
            } /*ctor*/

            /* build KF observable object where keeping all observations */
            static KalmanFilterObservable keep_all(MatrixXd H, MatrixXd R);

            /* build KF observable object.  replace H, R with reindexed versions H', R'
             * according to indexing vector keep[].   keep[] indexes members of
             * observation vector z(k)[j].   Reindexed z', H', R' as follows:
             *
             *   z'[j*]       = z[keep[j*]]
             *   H'[j*, i]    = H[keep[j*], i]
             *   R'[j1*, j2*] = R[keep[j1*], keep[j2*]]
             */
            static KalmanFilterObservable reindex(VectorXi keep, MatrixXd H, MatrixXd R);

            uint32_t n_state() const { return H_.cols(); }
            uint32_t n_observable() const { return H_.rows(); }
            VectorXi const & keep() const { return keep_; }
            MatrixXd const & observable() const { return H_; }
            MatrixXd const & observable_cov() const { return R_; }

            bool check_ok() const {
                uint32_t m = H_.rows();
                bool keep_is_mx1 = (keep_.rows() == m);
                bool r_is_mxm = ((R_.cols() == m) && (R_.rows() == m));

                bool keep_is_well_ordered = true;

                /* members of .keep are non-negative integers,
                 * in strictly increasing order
                 */
                int64_t keep_jm1 = -1;

                for (uint32_t j = 0; j < keep_.rows(); ++j) {
                    if (keep_[j] < 0)
                        keep_is_well_ordered = false;
                    if (keep_[j] <= keep_jm1)
                        keep_is_well_ordered = false;
                }

                /* also would like to require: R is +ve definite */

                return keep_is_mx1 && keep_is_well_ordered && r_is_mxm;
            } /*check_ok*/

            void display(std::ostream & os) const;
            std::string display_string() const;

        private:

        private:
            /* m: #of observations that survived sanity/error checks
             *
             * H, R here will have been re-indexed to exclude rejected observations.
             * observations z will also have been re-indexed.
             *
             * If an observation z[j] is excluded,  then also exclude:
             * - j'th row of H
             * - j'th row and j'th column of R
             * - j'th element of z
             *
             * Given re-indexed H, R,   the j*'th row of H goes with z[keep[j*]]
             */

            /* [m x 1] maps back to indices in original observation vector */
            VectorXi keep_;
            /* [m x n] observation matrix */
            MatrixXd H_;
            /* [m x m] covariance matrix for observation noise */
            MatrixXd R_;
        }; /*KalmanFilterObservable*/

        inline std::ostream &
        operator<<(std::ostream & os, KalmanFilterObservable const & x)
        {
            x.display(os);
            return os;
        } /*operator<<*/
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterObservable.hpp */
