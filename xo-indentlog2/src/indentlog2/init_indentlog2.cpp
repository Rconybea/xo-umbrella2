/** @file init_indentlog2.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "init_indentlog2.hpp"
#include <xo/ppsink/scope.hpp>

namespace xo {
    void
    InitSubsys<S_indentlog2_tag>::init()
    {
        /* Runtime initialization for xo-indentlog2.
         */
    }

    InitEvidence
    InitSubsys<S_indentlog2_tag>::require()
    {
        InitEvidence retval;

        retval ^= Subsystem::provide<S_indentlog2_tag>("indentlog2", &init);

        return retval;
    }
}

/* end init_indentlog2.cpp */
