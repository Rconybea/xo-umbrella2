/* @file KalmanFilterState.cpp */

#include "KalmanFilterState.hpp"
#include "Eigen/src/Core/Matrix.h"
#include "print_eigen.hpp"
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflect/TaggedPtr.hpp>
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <xo/ppsink/pp_time.hpp>      /* Prettifier<utc_nanos>: keeps xo's space-free format */
#include <ostream>
#include <string>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedRcptr;
    using xo::reflect::StructReflector;
    using xo::time::utc_nanos;
    using xo::rp;
    using xo::pp::matrix;
    //using xo::pp::vector;
    //using xo::scope;
    //using Eigen::LDLT;
    using Eigen::MatrixXd;
    using Eigen::VectorXd;

    namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
        using xo::pp::tostr;
        using xo::pp::xtag;

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

#ifdef OBSOLETE
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
#endif

        void
        KalmanFilterState::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            sink.pretty_struct("KalmanFilterState",
                               field("k", k_),
                               field("tk", tk_),
                               field("x", matrix(x_)),
                               field("P", matrix(P_)));
        }

        std::string
        KalmanFilterState::display_string() const
        {
            return xo::pp::tostr(*this);
        } /*display_string*/

        // ----- KalmanFilterStateExt -----

        rp<KalmanFilterStateExt>
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

#ifdef OBSOLETE
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
#endif

        void
        KalmanFilterStateExt::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            /* field() captures BY REFERENCE and these accessors return BY
             * VALUE, so each needs a named local -- see field_impl's lifetime
             * rule in pretty_struct.hpp.  matrix() likewise refers to its
             * argument, so the matrices need locals too.
             */
            const auto k = this->step_no();
            const auto tk = this->tm();
            const auto & x = this->state_v();
            const auto & P = this->state_cov();

            sink.pretty_struct("KalmanFilterStateExt",
                               field("k", k),
                               field("tk", tk),
                               field("x", matrix(x)),
                               field("P", matrix(P)),
                               field("K", matrix(K_)),
                               field("j", j_));
        }

        TaggedRcptr
        KalmanFilterStateExt::self_tp()
        {
            return Reflect::make_rctp(this);
        } /*self_tp*/
    } /*namespace filter*/
} /*namespace xo*/

/* end KalmanFilterState.cpp */
