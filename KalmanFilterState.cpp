/* @file KalmanFilterState.cpp */

#include "KalmanFilterState.hpp"
#include "print_eigen.hpp"
#include "reflect/StructReflector.hpp"
#include "reflect/TaggedPtr.hpp"
#include "indentlog/scope.hpp"
#include "Eigen/src/Core/Matrix.h"
#include <ostream>
#include <string>

namespace xo {
  using xo::reflect::Reflect;
  using xo::reflect::TaggedRcptr;
  using xo::reflect::StructReflector;
  using xo::time::utc_nanos;
  using xo::ref::rp;
  using logutil::matrix;
  using logutil::vector;
  //using xo::scope;
  using xo::xtag;
  using xo::tostr;
  //using Eigen::LDLT;
  using Eigen::MatrixXd;
  using Eigen::VectorXd;

  namespace kalman {
    // ----- KalmanFilterState -----

    rp<KalmanFilterState>
    KalmanFilterState::make()
    {
      return new KalmanFilterState();
    } /*make*/

    rp<KalmanFilterState>
    KalmanFilterState::make(uint32_t k,
                utc_nanos tk,
                VectorXd x,
                MatrixXd P,
                KalmanFilterTransition transition)
    {
      return new KalmanFilterState(k, tk,
                   std::move(x),
                   std::move(P),
                   std::move(transition));
    } /*make*/

    void
    KalmanFilterState::reflect_self()
    {
      StructReflector<KalmanFilterState> sr;

      if (sr.is_incomplete()) {
    REFLECT_MEMBER(sr, k);
    REFLECT_MEMBER(sr, tk);
    REFLECT_MEMBER(sr, x);
    REFLECT_MEMBER(sr, P);
      }
    } /*reflect_self*/

    KalmanFilterState::KalmanFilterState() = default;

    KalmanFilterState::KalmanFilterState(uint32_t k,
                     utc_nanos tk,
                     VectorXd x,
                     MatrixXd P,
                     KalmanFilterTransition transition)
      : k_{k}, tk_{tk},
    x_{std::move(x)}, P_{std::move(P)},
    transition_{std::move(transition)}
    {}

    TaggedRcptr
    KalmanFilterState::self_tp()
    {
      return Reflect::make_rctp(this);
    } /*self_tp*/

    // ----- KalmanFilterExt -----

    rp<KalmanFilterStateExt>
    KalmanFilterStateExt::make()
    {
      return new KalmanFilterStateExt();
    } /*make*/

    rp<KalmanFilterStateExt>
    KalmanFilterStateExt::make(uint32_t k,
                   utc_nanos tk,
                   VectorXd x,
                   MatrixXd P,
                   KalmanFilterTransition transition,
                   MatrixXd K,
                   int32_t j,
                   rp<KalmanFilterInput> zk)
    {
      return new KalmanFilterStateExt(k,
                      tk,
                      std::move(x),
                      std::move(P),
                      std::move(transition),
                      std::move(K),
                      j,
                      std::move(zk));
    } /*make*/

    void
    KalmanFilterStateExt::reflect_self()
    {
      StructReflector<KalmanFilterStateExt> sr;

      if (sr.is_incomplete()) {
    /* TODO: use sr.adopt_ancestors<KalmanFilterState>() */

    REFLECT_EXPLICIT_MEMBER(sr, "k", &KalmanFilterState::k_);
    REFLECT_EXPLICIT_MEMBER(sr, "tk", &KalmanFilterState::tk_);
    REFLECT_EXPLICIT_MEMBER(sr, "x", &KalmanFilterState::x_);
    REFLECT_EXPLICIT_MEMBER(sr, "P", &KalmanFilterState::P_);
    REFLECT_EXPLICIT_MEMBER(sr, "transition", &KalmanFilterState::transition_);
    REFLECT_MEMBER(sr, j);
    REFLECT_MEMBER(sr, K);
    REFLECT_MEMBER(sr, zk);
      }
    } /*reflect_self*/

    KalmanFilterStateExt::KalmanFilterStateExt(uint32_t k,
                           utc_nanos tk,
                           VectorXd x,
                           MatrixXd P,
                           KalmanFilterTransition transition,
                           MatrixXd K,
                           int32_t j,
                           rp<KalmanFilterInput> zk)
      : KalmanFilterState(k, tk,
              std::move(x),
              std::move(P),
              std::move(transition)),
    j_{j},
    K_{std::move(K)},
    zk_{std::move(zk)}
    {
      uint32_t n = x.size();

      if (n != P.rows() || n != P.cols()) {
    std::string err_msg
      = tostr("with n=x.size expect [n x n] covar matrix P",
          xtag("n", x.size()),
          xtag("P.rows", P.rows()),
          xtag("P.cols", P.cols()));

      throw std::runtime_error(err_msg);
      }

      if ((K.rows() > 0) && (K.rows() > 0)) {
    if (n != K.rows()) {
      std::string err_msg
        = tostr("with n=x.size expect [m x n] gain matrix K",
            xtag("n", x.size()),
            xtag("K.rows", K.rows()),
            xtag("K.cols", K.cols()));

      throw std::runtime_error(err_msg);
    }
      } else {
    /* bypass test with [0 x 0] matrix K;
     * normal for initial filter state
     */
      }
    } /*ctor*/

    void
    KalmanFilterState::display(std::ostream & os) const
    {
      os << "<KalmanFilterState"
     << xtag("k", k_)
     << xtag("tk", tk_)
     << xtag("x", matrix(x_))
     << xtag("P", matrix(P_))
     << ">";
    } /*display*/

    std::string
    KalmanFilterState::display_string() const
    {
      std::stringstream ss;
      ss << *this;
      return ss.str();
    } /*display_string*/

    // ----- KalmanFilterStateExt -----

    ref::rp<KalmanFilterStateExt>
    KalmanFilterStateExt::initial(utc_nanos t0,
                  VectorXd x0,
                  MatrixXd P0)
    {
      return KalmanFilterStateExt::make
    (0 /*k*/,
     t0,
     std::move(x0),
     std::move(P0),
     KalmanFilterTransition(MatrixXd() /*F - not used for initial step*/,
                MatrixXd() /*Q - not used for initial step*/),
     MatrixXd() /*K - not used for initial step*/,
     -1 /*j - not used for initial step*/,
     nullptr /*zk - not defined for initial step*/);
    } /*initial*/

    void
    KalmanFilterStateExt::display(std::ostream & os) const
    {
      os << "<KalmanFilterStateExt"
     << xtag("k", this->step_no())
     << xtag("tk", this->tm())
     << xtag("x", matrix(this->state_v()))
     << xtag("P", matrix(this->state_cov()))
     << xtag("K", matrix(K_))
     << xtag("j", j_)
     << ">";
    } /*display*/

    TaggedRcptr
    KalmanFilterStateExt::self_tp()
    {
      return Reflect::make_rctp(this);
    } /*self_tp*/
  } /*namespace filter*/
} /*namespace xo*/

/* end KalmanFilterState.cpp */
