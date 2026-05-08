/** @file GCObjectConversion.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "GCObjectConversion.hpp"

namespace xo {
    using xo::reflect::typeseq;

    namespace scm {

        void
        GCObjectConversionUtil::_from_gco_fail_aux(obj<AGCObject> gco,
                                                   typeseq tseq,
                                                   scope * p_log)
        {
            p_log->retroactively_enable();
            if (p_log) {
                (*p_log)(xtag("gco.tseq", gco._typeseq()));
                (*p_log)(xtag("DRepr.tseq", tseq));
            }
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end GCObjectConversion.cpp */
