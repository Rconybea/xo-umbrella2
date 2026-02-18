/** @file init_procedure2.cpp
 *
 *  @author Roland Conybeare, Jan 2026
**/

#include "init_procedure2.hpp"
#include "init_primitives.hpp"
#include "procedure2_register_facets.hpp"
#include "procedure2_register_types.hpp"

#include <xo/object2/init_object2.hpp>
#include <xo/gc/CollectorTypeRegistry.hpp>

namespace xo {
    using xo::scm::procedure2_register_facets;
    using xo::scm::procedure2_register_types;
    using xo::mm::CollectorTypeRegistry;

    void
    InitSubsys<S_procedure2_tag>::init()
    {
        procedure2_register_facets();

        CollectorTypeRegistry::instance().register_types(&procedure2_register_types);
    }

    InitEvidence
    InitSubsys<S_procedure2_tag>::require()
    {
        InitEvidence retval;

        /* recursive subsystem deps for xo-object2/ */
        retval ^= InitSubsys<S_object2_tag>::require();

        /* xo-procedure2/'s own initialization code */
        retval ^= Subsystem::provide<S_procedure2_tag>("procedure2", &init);

        return retval;
    }
} /*namespace xo*/

/* end init_procedure2.cpp */
