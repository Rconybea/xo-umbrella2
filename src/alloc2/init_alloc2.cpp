/** @file init_alloc2.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "init_alloc2.hpp"
#include "alloc2_register_facets.hpp"

namespace xo {
    using xo::mm::alloc2_register_facets;
    // using xo::mm::alloc2_register_types;
    // using xo::mm::CollectorTypeRegistry;

    void
    InitSubsys<S_alloc2_tag>::init()
    {
        alloc2_register_facets();
    }

    InitEvidence
    InitSubsys<S_alloc2_tag>::require()
    {
        InitEvidence retval;

        /* direct subsystem deps for xo-alloc2/ (if/when) */
        //retval ^= InitSubsys<S_foo_tag>>::require();

        /* xo-alloc2/'s own initialization code */
        retval ^= Subsystem::provide<S_alloc2_tag>("alloc2", &init);

        return retval;
    }

} /*namespace xo*/

/* end init_alloc2.cpp */
