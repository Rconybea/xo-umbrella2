/* @file KalmanFilterObservable.cpp */

#include "KalmanFilterObservable.hpp"
#include "print_eigen.hpp"
#include "indentlog/scope.hpp"

namespace xo {
  using xo::scope;
  using logutil::matrix;
  using xo::xtag;

  namespace kalman {
    KalmanFilterObservable
    KalmanFilterObservable::keep_all(MatrixXd H,
                     MatrixXd R)
    {
      VectorXi keep(H.rows());

      for (uint32_t j=0; j<H.rows(); ++j)
    keep[j] = j;

      return KalmanFilterObservable(std::move(keep),
                    std::move(H),
                    std::move(R));
    } /*keep_all*/

    KalmanFilterObservable
    KalmanFilterObservable::reindex(VectorXi keep,
                                    MatrixXd H,
                                    MatrixXd R)
    {
        scope log(XO_DEBUG(false /*debug_flag*/));

        /* Hp:
         *   - keep rows in H with indices that appear in keep[]
         *   - keep all columns of H
         */

        MatrixXd Hp = H(keep, Eigen::all);
        MatrixXd Rp = R(keep, keep);

        if (log.enabled()) {
            log(xtag("keep", matrix(keep)));
            log(xtag("H", matrix(H)));
            log(xtag("R", matrix(R)));
        }

        return KalmanFilterObservable(keep, Hp, Rp);
    } /*reindex*/

      void
      KalmanFilterObservable::display(std::ostream & os) const
      {
          os << "<KalmanFilterObservable"
             << xtag("H", matrix(H_))
             << xtag("R", matrix(R_))
             << ">";
      } /*display*/

      std::string
      KalmanFilterObservable::display_string() const
      {
          std::stringstream ss;
          this->display(ss);
          return ss.str();
      } /*display_string*/

  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterObservable.cpp */
