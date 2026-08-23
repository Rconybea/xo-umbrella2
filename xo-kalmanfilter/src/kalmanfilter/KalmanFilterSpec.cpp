/* @file KalmanFilterSpec.cpp */

#include "KalmanFilterSpec.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {

    namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
        using xo::pp::tostr;
        using xo::pp::xtag;

#ifdef OBSOLETE
        void
        KalmanFilterSpec::display(std::ostream & os) const
        {
            os << "<KalmanFilterSpec"
               << xtag("start_ext", start_ext_)
               << ">";
        } /*display*/
#endif

        void
        KalmanFilterSpec::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            sink.pretty_struct("KalmanFilterSpec",
                               field("start_ext", start_ext_));
        }

        std::string
        KalmanFilterSpec::display_string() const
        {
            return tostr(*this);
        } /*display_string*/
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterSpec.cpp */
