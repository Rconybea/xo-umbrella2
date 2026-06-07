/* @file ExpProcess.cpp */

#include "xo/reflect/TaggedPtr.hpp"
#include "xo/reflect/StructReflector.hpp"
//#include "time/Time.hpp"
#include "ExpProcess.hpp"

namespace xo {
    using reflect::Reflect;
    using reflect::StructReflector;
    using reflect::TaggedRcptr;
    using xo::scope;
    using xo::xtag;

    namespace process {
        void
        ExpProcess::reflect_self()
        {
            StructReflector<ExpProcess> sr;

            if (sr.is_incomplete()) {
                REFLECT_MEMBER(sr, scale);
                REFLECT_MEMBER(sr, exponent_process);
            }
        } /*self_reflect*/

        /* note: lo is a sample from the exponentiated process;
         *       must take log to get sample from the exponent process
         */
        ExpProcess::value_type
        ExpProcess::exterior_sample(utc_nanos t,
                                    event_type const & lo)
        {
            constexpr bool c_logging_enabled = false;

            scope log(XO_DEBUG(c_logging_enabled));

            double lo_value = lo.second;
            double log_lo_value = ::log(lo.second / this->scale_);

            double e
                = (this->exponent_process_->exterior_sample
                   (t,
                    event_type(lo.first, log_lo_value)));

            double retval = this->scale_ * ::exp(e);

            log && log("result",
                       xtag("t", t),
                       xtag("lo.tm", lo.first),
                       xtag("lo.value", lo_value),
                       xtag("log(lo.value/m)", log_lo_value),
                       xtag("m", this->scale_),
                       xtag("e", e),
                       xtag("retval", retval));

            return retval;
        } /*exterior_sample*/

        TaggedRcptr
        ExpProcess::self_tp()
        {
            return Reflect::make_rctp(this);
        } /*self_tp*/

    } /*namespace process*/
} /*namespace xo*/

/* end ExpProcess.cpp */
