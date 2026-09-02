/** @file init_indentlog2.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "init_indentlog2.hpp"
#include "SetupIndentlog2.hpp"
#include <xo/ppsink/scope.hpp>

namespace xo {
    using xo::SetupIndentlog2;
    using xo::pp::scope;

    void
    InitSubsys<S_indentlog2_tag>::init()
    {
        /* Runtime initialization for xo-indentlog2.
         */
    }

    InitEvidence
    InitSubsys<S_indentlog2_tag>::require()
    {
        scope log(XO_DEBUG_(true));

        InitEvidence retval;

        retval ^= Subsystem::provide<S_indentlog2_tag>("indentlog2", &init);

        return retval;
    }
}

/* end init_indentlog2.cpp */
