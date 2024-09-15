/* @file KalmanFilterEngine.cpp
 *
 */

#include "KalmanFilterEngine.hpp"
#include "print_eigen.hpp"
#include "xo/indentlog/scope.hpp"
#include "Eigen/src/Core/Matrix.h"

namespace xo {
    using xo::time::utc_nanos;
    using logutil::matrix;
    using xo::scope;
    using xo::xtag;
    using Eigen::LDLT;
    using Eigen::MatrixXd;
    using Eigen::VectorXd;

    namespace kalman {
        // ----- KalmanFilterEngine -----

        rp<KalmanFilterState>
        KalmanFilterEngine::extrapolate(utc_nanos tkp1,
                                        rp<KalmanFilterState> const & s,
                                        KalmanFilterTransition const & f)
        {
            //constexpr char const * c_self_name
            //    = "KalmanFilterEngine::extrapolate";

            /* prior estimates at t(k) */
            VectorXd const & x = s->state_v();
            MatrixXd const & P = s->state_cov();

            /* model change from t(k) -> t(k+1) */
            MatrixXd const & F = f.transition_mat();
            MatrixXd const & Q = f.transition_cov();

            if(F.cols() != x.rows()) {
                scope log(XO_DEBUG(true /*debug_flag*/));

                log("error: F*x: expected F.cols=x.rows",
                    xtag("F.cols", F.cols()), xtag("x.rows", x.rows()));
            }

            /* x(k+1|k) */
            VectorXd x_ext = F * x;

            /* P(k+1|k) */
            MatrixXd P_ext = (F * P * F.transpose()) + Q;

            /* creating new state object here
             * allows KalmanFilterSvc.is_volatile()=false
             */

            return KalmanFilterState::make(s->step_no() + 1,
                                           tkp1,
                                           std::move(x_ext),
                                           std::move(P_ext),
                                           f);
        } /*extrapolate*/

        VectorXd
        KalmanFilterEngine::kalman_gain1(rp<KalmanFilterState> const & skp1_ext,
                                         KalmanFilterObservable const & h,
                                         uint32_t j)
        {
            constexpr bool c_debug_enabled = false;

            scope log(XO_DEBUG(c_debug_enabled));

            /* P(k+1|k) :: [n x n] */
            MatrixXd const & P_ext = skp1_ext->state_cov();

            /* H(k) :: [m x n] */
            MatrixXd const & H = h.observable();
            /* R(k) :: [m x m] */
            MatrixXd const & R = h.observable_cov();

            /* i'th col of H couples element #i of filter state to each member of input z(k);
             * j'th row of H couples filter state to j'th observable
             *
             * Hj :: [1 x n]    Hj is a row-vector
             */
            auto Hj = H.row(j);

            /* Rjj is the j'th diagonal element of R */
            double Rjj = R(j, j);

            /*                            T
             *   M(k) = Hj * P(k+1|k) * Hj  + Rjj
             *
             * M(k) is a [1 x 1] matrix
             */
            double m = Hj * (P_ext * Hj.transpose()) + Rjj;

            /*     -1
             * M(k)      trivial,  since M is [1 x 1]
             */
            double m_inv = 1.0 / m;

            /* K :: [n x 1] */
            VectorXd K = P_ext * Hj.transpose() * m_inv;

            log && log("result",
                       xtag("P(k+1|k)", matrix(P_ext)),
                       xtag("R", matrix(R)),
                       xtag("m", m));

            return K;
        } /*kalman_gain1*/

        MatrixXd
        KalmanFilterEngine::kalman_gain(rp<KalmanFilterState> const & skp1_ext,
                                        KalmanFilterObservable const & h)
        {
            scope log(XO_DEBUG(false /*debug_enabled*/));

            /* P(k+1|k) */
            MatrixXd const & P_ext = skp1_ext->state_cov();

            MatrixXd const & H = h.observable();
            MatrixXd const & R = h.observable_cov();

            uint32_t m = H.rows();
            uint32_t n = H.cols();

            if ((P_ext.rows() != n) || (P_ext.cols() != n)) {
                std::string err_msg
                    = tostr("kalman_gain: with dim(H) = [m x n] expect dim(P) = [n x n]",
                            xtag("m", m), xtag("n", n),
                            xtag("P.rows", P_ext.rows()),
                            xtag("P.cols", P_ext.cols()));

                throw std::runtime_error(err_msg);
            }

            if ((R.rows() != m) || (R.cols() != m)) {
                std::string err_msg
                    = tostr("kalman_gain: with dim(H) = [m x n] expect dim(R) = [m x m]",
                            xtag("m", m), xtag("n", n),
                            xtag("R.rows", R.rows()), xtag("R.cols", R.cols()));

                throw std::runtime_error(err_msg);
            }

            /* kalman gain:
             *                         T  -1
             *   K(k+1) = P(k+1|k).H(k) .M
             *
             *                         T /                   T       \ -1
             *          = P(k+1|k).H(k) .| H(k).P(k+1|k).H(k) + R(k) |
             *                           \                           /
             *
             * Notes:
             * 1. the matrix M being inverted is symmetric,  since represents covariances.
             * 2. if diagonal of R(k) has no zeroes (i.e. all measurements are subject to error),
             *    then it must be non-negative definite
             * 3. unless observation errors are perfectly correlated, M(k)
             *    is positive definite.
             * 4. even though 3. holds,  there may be a nearby non-positive-definite matrix M+dM,
             *    Factoring M with finite-precision arithmetic solves for M+dM instead of M;
             *    which may run into difficulty if M is only 'slighlty' +ve definite.
             *    If necessary add small diagonal correction D to M,
             *    sufficient to make M+D positive definite.
             *    This is equivalent to introducing additional
             *    uncorrelated observation error,   so benign from a robustness perspective
             * 5. In generally we usually want to avoid fully realizing a matrix inverse.
             *    In this case need to explicitly compute K as ingredient used to
             *    correct state covariance later.
             * 6. However,  if R is diagonal (which is in practice quite likely),
             *    then it's easy to decompose a suite of vector observations z(k+1) = [z1, ..zm]T
             *    into separate zi, with dt=0 separating them.
             *    Can use this to avoid computing the inverse.
             *    See .kalman_gain1(), .correct1()
             * 7. .kalman_gain() works unaltered when H, R have been reindexed
             *    to exclude outliers/errors;   this is true because .kalman_gain() does not
             *    use the observation vector z[], i.e. operates entirely in the reduced
             *    reindexed space.
             */

            MatrixXd M = H * P_ext * H.transpose() + R;

            /* will use to write M as:
             *
             *        T      T
             *   M = P .L.D.L .P
             *
             * where:
             *   P is a permutation matrix
             *   L is lower triangular,  with unit diagonal
             *   D is diagonal
             */
            LDLT<MatrixXd> ldlt = M.ldlt();

            /* solve for the identity matrix to realize the inverse this way */
            MatrixXd I = MatrixXd::Identity(M.rows(), M.cols());

            /*  -1
             * M
             */
            MatrixXd M_inv = ldlt.solve(I);

            /* K(k+1) */
            MatrixXd K = P_ext * H.transpose() * M_inv;

            log && log("result",
                       xtag("k", skp1_ext->step_no()),
                       xtag("P(k+1|k)", matrix(P_ext)),
                       xtag("H", matrix(H)),
                       xtag("R", matrix(R)),
                       xtag("M", matrix(M)),
                       xtag("K", matrix(K)));

            return K;
        } /*kalman_gain*/

        rp<KalmanFilterStateExt>
        KalmanFilterEngine::correct1(rp<KalmanFilterState> const & skp1_ext,
                                     KalmanFilterObservable const & h,
                                     rp<KalmanFilterInput> const & zkp1,
                                     uint32_t j)
        {
            uint32_t n = skp1_ext->n_state();
            /* Kj :: [n x 1] */
            VectorXd Kj = kalman_gain1(skp1_ext, h, j);
            /* H :: [m x n] */
            MatrixXd const & H = h.observable();
            VectorXd const & z = zkp1->z();

            /* Hj :: [1 x n]  the j'th row of H */
            auto const & Hj = H.row(j);


            /* x(k+1|x) :: [n x 1] */
            VectorXd const & x_ext = skp1_ext->state_v();

            /* P(k+1|k) :: [n x n] */
            MatrixXd const & P_ext = skp1_ext->state_cov();

            /* innovj : difference between jth 'actual observation'
             *          and jth 'predicted observation'
             */
            double innovj = z[j] - (Hj * x_ext);

            /* x(k+1) */
            VectorXd xkp1 = x_ext + (Kj * innovj);

            MatrixXd I = MatrixXd::Identity(n, n);
            /* note: Kj [n x 1], Hj [1 x n],
             *       so Kj * Hj [n x n],  with rank 1
             */
            MatrixXd Pkp1 = (I - (Kj * Hj)) * P_ext;

            return KalmanFilterStateExt::make(skp1_ext->step_no(),
                                              skp1_ext->tm(),
                                              xkp1,
                                              Pkp1,
                                              skp1_ext->transition(),
                                              Kj,
                                              j,
                                              zkp1);
        } /*correct1*/

        rp<KalmanFilterStateExt>
        KalmanFilterEngine::correct(rp<KalmanFilterState> const & skp1_ext,
                                    KalmanFilterObservable const & h,
                                    rp<KalmanFilterInput> const & zkp1)
        {
            uint32_t n = skp1_ext->n_state();
            /* K :: [n x m] */
            MatrixXd K = kalman_gain(skp1_ext, h);
            MatrixXd const & H = h.observable();
            /* z_orig[] is original observation vector before reindexing */
            VectorXd const & z_orig = zkp1->z();
            /* reindex z_orig, keeping only elements that appear in
             */
            VectorXd z = z_orig(h.keep());

            /* 'ext' short for 'extrapolated' */
            VectorXd const & x_ext = skp1_ext->state_v();
            MatrixXd const & P_ext = skp1_ext->state_cov();

            /* innov: difference between 'actual observations'
             * and 'predicted observations'
             */
            VectorXd innov = z - (H * x_ext);

            /* x(k+1) :: [n x 1] */
            VectorXd xkp1 = x_ext + K * innov;
            MatrixXd I = MatrixXd::Identity(n, n);
            MatrixXd Pkp1 = (I - K * H) * P_ext;

            return KalmanFilterStateExt::make(skp1_ext->step_no(),
                                              skp1_ext->tm(),
                                              xkp1,
                                              Pkp1,
                                              skp1_ext->transition(),
                                              K,
                                              -1 /*j: not used*/,
                                              zkp1);
        } /*correct*/

        rp<KalmanFilterStateExt>
        KalmanFilterEngine::step(utc_nanos tkp1,
                                 rp<KalmanFilterState> const & sk,
                                 KalmanFilterTransition const & Fk,
                                 KalmanFilterObservable const & Hkp1,
                                 rp<KalmanFilterInput> const & zkp1)
        {
            rp<KalmanFilterState> skp1_ext
                = KalmanFilterEngine::extrapolate(tkp1, sk, Fk);

            rp<KalmanFilterStateExt> skp1
                = KalmanFilterEngine::correct(skp1_ext, Hkp1, zkp1);

            return skp1;
        } /*step*/

        rp<KalmanFilterStateExt>
        KalmanFilterEngine::step(KalmanFilterStep const & step_spec)
        {
            return step(step_spec.tkp1(),
                        step_spec.state(),
                        step_spec.model(),
                        step_spec.obs(),
                        step_spec.input());
        } /*step*/

        rp<KalmanFilterStateExt>
        KalmanFilterEngine::step1(utc_nanos tkp1,
                                  rp<KalmanFilterState> const & sk,
                                  KalmanFilterTransition const & Fk,
                                  KalmanFilterObservable const & Hkp1,
                                  rp<KalmanFilterInput> const & zkp1,
                                  uint32_t j)
        {
            rp<KalmanFilterState> skp1_ext
                = KalmanFilterEngine::extrapolate(tkp1, sk, Fk);

            rp<KalmanFilterStateExt> skp1
                = KalmanFilterEngine::correct1(skp1_ext, Hkp1, zkp1, j);

            return skp1;
        } /*step1*/

        rp<KalmanFilterStateExt>
        KalmanFilterEngine::step1(KalmanFilterStep const & step_spec,
                                  uint32_t j)
        {
            return step1(step_spec.tkp1(),
                         step_spec.state(),
                         step_spec.model(),
                         step_spec.obs(),
                         step_spec.input(),
                         j);
        } /*step1*/
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterEngine.cpp */
