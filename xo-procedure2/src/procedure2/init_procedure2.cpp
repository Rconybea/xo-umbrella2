/** @file init_procedure2.cpp
 *
 *  @author Roland Conybeare, Jan 2026
**/

#include "init_procedure2.hpp"
#include "init_primitives.hpp"
#include "SetupProcedure2.hpp"
#include <xo/object2/init_object2.hpp>
#include <xo/type/init_type.hpp>
#include <xo/alloc2/CollectorTypeRegistry.hpp>

namespace xo {
    using xo::scm::SetupProcedure2;
    using xo::scm::PrimitiveRegistry;
    using xo::mm::CollectorTypeRegistry;

    void
    InitSubsys<S_procedure2_tag>::init()
    {
        SetupProcedure2::register_facets();

        CollectorTypeRegistry::instance().register_types(&SetupProcedure2::register_types);
        PrimitiveRegistry::instance().register_primitives(&SetupProcedure2::register_primitives);
    }

    InitEvidence
    InitSubsys<S_procedure2_tag>::require()
    {
        InitEvidence retval;

        /* recursive subsystem deps for xo-object2/ */
        retval ^= InitSubsys<S_object2_tag>::require();
        retval ^= InitSubsys<S_type_tag>::require();

        /* xo-procedure2/'s own initialization code */
        retval ^= Subsystem::provide<S_procedure2_tag>("procedure2", &init);

        return retval;
    }
} /*namespace xo*/

/* end init_procedure2.cpp */
