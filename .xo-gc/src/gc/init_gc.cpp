/** @file init_gc.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "init_gc.hpp"
#include "setup_gc.hpp"
#include <xo/object2/init_object2.hpp>
#include <xo/alloc2/init_alloc2.hpp>

namespace xo {
    using xo::mm::SetupGc;

    void
    InitSubsys<S_gc_tag>::init()
    {
        SetupGc::register_facets();
    }

    InitEvidence
    InitSubsys<S_gc_tag>::require() {
        InitEvidence retval;

        /* recursive subsystem deps for xo-gc/ */
        retval ^= InitSubsys<S_object2_tag>::require();
        retval ^= InitSubsys<S_alloc2_tag>::require();

        /* xo-gc/'s own initialization code */
        retval ^= Subsystem::provide<S_gc_tag>("gc", &init);

        return retval;
    }

} /*namespace xo*/

/* end init_gc.cpp */
