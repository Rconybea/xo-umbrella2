/* @file KalmanFilterObservable.cpp */

#include "KalmanFilterObservable.hpp"
#include "print_eigen.hpp"
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */
#include <xo/indentlog2/print/tostr.hpp>  /* xo::pp::tostr */

namespace xo {
    using xo::pp::matrix;

  namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
      using xo::pp::scope;
      using xo::pp::xtag;

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
        scope log(XO_DEBUG_(false /*debug_flag*/));

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

#ifdef OBSOLETE
      void
      KalmanFilterObservable::display(std::ostream & os) const
      {
          os << "<KalmanFilterObservable"
             << xtag("H", matrix(H_))
             << xtag("R", matrix(R_))
             << ">";
      } /*display*/
#endif

        void
        KalmanFilterObservable::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            sink.pretty_struct("KalmanFilterObservable",
                               field("H", matrix(H_)),
                               field("R", matrix(R_)));
        }

      std::string
      KalmanFilterObservable::display_string() const
      {
          return xo::pp::tostr(*this);
      } /*display_string*/

  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterObservable.cpp */
