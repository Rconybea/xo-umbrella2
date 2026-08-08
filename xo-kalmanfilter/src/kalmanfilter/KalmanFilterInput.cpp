/* @file KalmanFilterInput.cpp */

#include "KalmanFilterInput.hpp"
#include "Eigen/src/Core/Matrix.h"
#include "print_eigen.hpp"
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflect/TaggedRcptr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <xo/ppsink/pp_time.hpp>      /* Prettifier<utc_nanos>: keeps xo's space-free format */

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedRcptr;
    using xo::reflect::StructReflector;
    using logutil::matrix;
    using Eigen::MatrixXd;
    using Eigen::VectorXi;
    using std::uint32_t;

    namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
        using xo::pp::scope;
        using xo::pp::xtag;

        rp<KalmanFilterInput>
        KalmanFilterInput::make(utc_nanos tkp1,
                                VectorXb const & presence,
                                VectorXd const & z,
                                VectorXd const & Rd)
        {
            return new KalmanFilterInput(tkp1, presence, z, Rd);
        } /*make*/

        rp<KalmanFilterInput>
        KalmanFilterInput::make_present(utc_nanos tkp1,
                                        VectorXd const & z)
        {
            VectorXb presence = VectorXb::Ones(z.cols());

            return new KalmanFilterInput(tkp1,
                                         presence,
                                         z,
                                         VectorXd(0) /*Rd - not using*/);
        } /*make*/

        VectorXi
        KalmanFilterInput::make_kept_index() const
        {
            scope log(XO_DEBUG_(false /*!debug_flag*/));

            log && log(xtag("presence", matrix(presence_)));

            /* count truth values */
            uint32_t mstar = 0;

            for (uint32_t j = 0, m = this->presence_.rows(); j<m; ++j) {
                if (this->presence_[j])
                    ++mstar;
            }

            log && log(xtag("m*", mstar));

            VectorXi keep(mstar);

            /* 2nd pass,  populate keep[] */

            uint32_t jstar = 0;

            for (uint32_t j = 0, m = this->presence_.rows(); j<m; ++j) {
                if (this->presence_[j]) {
                    keep[jstar] = j;
                    ++jstar;
                }
            }

            log && log("keep", matrix(keep));

            return keep;
        } /*make_kept_index*/

        void
        KalmanFilterInput::reflect_self()
        {
            StructReflector<KalmanFilterInput> sr;

            if (sr.is_incomplete()) {
                REFLECT_MEMBER(sr, tkp1);
                REFLECT_MEMBER(sr, presence);
                REFLECT_MEMBER(sr, z);
                REFLECT_MEMBER(sr, Rd);
            }
        } /*reflect_self*/

        reflect::TaggedRcptr
        KalmanFilterInput::self_tp()
        {
            return Reflect::make_rctp(this);
        } /*self_tp*/

        void
        KalmanFilterInput::display(std::ostream & os) const
        {
            os << "<KalmanFilterInput"
               << xtag("tkp1", tkp1_)
               << xtag("z", matrix(z_))
               << xtag("presence", matrix(presence_))
               << xtag("Rd", matrix(Rd_))
               << ">";
        } /*display*/

        std::string
        KalmanFilterInput::display_string() const
        {
            std::stringstream ss;
            this->display(ss);
            return ss.str();
        } /*display_string*/
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInput.cpp */
