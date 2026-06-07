/* @file KalmanFilterTransition.cpp */

#include "KalmanFilterTransition.hpp"
#include "print_eigen.hpp"
#include "xo/reflect/StructReflector.hpp"
#include "xo/indentlog/scope.hpp"

namespace xo {
    using xo::reflect::StructReflector;
    using logutil::matrix;
    using xo::xtag;

    namespace kalman {
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

        void
        KalmanFilterTransition::display(std::ostream & os) const
        {
            os << "<KalmanFilterTransition"
               << xtag("F", matrix(F_))
               << xtag("Q", matrix(Q_))
               << ">";
        } /*display*/

        std::string
        KalmanFilterTransition::display_string() const
        {
            std::stringstream ss;
            this->display(ss);
            return ss.str();
        } /*display_string*/

    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterTransition.cpp */
