/** @file init_alloc2.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "init_alloc2.hpp"
#include "SetupAlloc2.hpp"
#include <xo/facet/init_facet.hpp>

namespace xo {
    using xo::mm::SetupAlloc2;
    // using xo::mm::alloc2_register_types;
    // using xo::mm::CollectorTypeRegistry;

    void
    InitSubsys<S_alloc2_tag>::init()
    {
        SetupAlloc2::register_facets();
    }

    InitEvidence
    InitSubsys<S_alloc2_tag>::require()
    {
        InitEvidence retval;

        /* direct subsystem deps for xo-alloc2/ */
        retval ^= InitSubsys<S_facet_tag>::require();

        /* xo-alloc2/'s own initialization code */
        retval ^= Subsystem::provide<S_alloc2_tag>("alloc2", &init);

        return retval;
    }

} /*namespace xo*/

/* end init_alloc2.cpp */
