/* @file KalmanFilterTransition.cpp */

#include "KalmanFilterTransition.hpp"
#include "print_eigen.hpp"
#include <xo/reflect/StructReflector.hpp>
#include <xo/indentlog2/print/tostr.hpp>  /* xo::pp::tostr */
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::reflect::StructReflector;
    using xo::pp::matrix;

    namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
        using xo::pp::xtag;

        void
        KalmanFilterTransition::reflect_self()
        {
            StructReflector<KalmanFilterTransition> sr;

            if (sr.is_incomplete()) {
                REFLECT_MEMBER(sr, F);
                REFLECT_MEMBER(sr, Q);
            }
        } /*reflect_self*/

        uint32_t
        KalmanFilterTransition::n_state() const
        {
            /* we know F.rows() == F.cols() = Q.cols() == Q.rows(),
             * see .check_ok()
             */

            return F_.rows();
        } /*n_state*/

#ifdef OBSOLETE
        void
        KalmanFilterTransition::display(std::ostream & os) const
        {
            os << "<KalmanFilterTransition"
               << xtag("F", matrix(F_))
               << xtag("Q", matrix(Q_))
               << ">";
        } /*display*/
#endif

        void
        KalmanFilterTransition::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            sink.pretty_struct("KalmanFilterTransition",
                               field("F", matrix(F_)),
                               field("Q", matrix(Q_)));
        }

        std::string
        KalmanFilterTransition::display_string() const
        {
            return xo::pp::tostr(*this);
        } /*display_string*/

    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterTransition.cpp */
