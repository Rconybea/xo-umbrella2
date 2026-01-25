/** @file init_procedure2.cpp
 *
 *  @author Roland Conybeare, Jan 2026
**/

#include "init_procedure2.hpp"
#include "init_primitives.hpp"
//#include "procedure2_register_facets.hpp"
//#include "procedure2_register_types.hpp"
#include <xo/gc/CollectorTypeRegistry.hpp>

namespace xo {
    using xo::scm::Primitives;

    void
    InitSubsys<S_procedure2_tag>::init()
    {
    }

    InitEvidence
    InitSubsys<S_procedure2_tag>::require()
    {
        InitEvidence retval;

        /* xo-procedure2/'s own initialization code */
        retval ^= Subsystem::provide<S_procedure2_tag>("procedure2", &init);

        return retval;
    }
} /*namespace xo*/

/* end init_procedure2.cpp */
