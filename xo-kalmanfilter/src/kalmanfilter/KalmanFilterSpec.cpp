/* @file KalmanFilterSpec.cpp */

#include "KalmanFilterSpec.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */

namespace xo {

    namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
        using xo::pp::tostr0;
        using xo::pp::xtag;

        void
        KalmanFilterSpec::display(std::ostream & os) const
        {
            os << "<KalmanFilterSpec"
               << xtag("start_ext", start_ext_)
               << ">";
        } /*display*/

        std::string
        KalmanFilterSpec::display_string() const
        {
            return tostr0(*this);
        } /*display_string*/
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterSpec.cpp */
