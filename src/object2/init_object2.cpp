/** @file init_object2.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "init_object2.hpp"
#include "object2_register_facets.hpp"
#include "object2_register_types.hpp"
#include <xo/gc/CollectorTypeRegistry.hpp>

namespace xo {
    using xo::scm::object2_register_facets;
    using xo::scm::object2_register_types;
    using xo::mm::CollectorTypeRegistry;

    void
    InitSubsys<S_object2_tag>::init()
        {
            object2_register_facets();

            CollectorTypeRegistry::instance().register_types(&object2_register_types);
        }

    InitEvidence
    InitSubsys<S_object2_tag>::require()
        {
            InitEvidence retval;

            /* direct subsystem deps for xo-object2/ */
            // retval ^= InitSubsys<S_somedep_tag>::require();

            /* xo-expression2/'s own initialization code */
            retval ^= Subsystem::provide<S_object2_tag>("object2", &init);

            return retval;
        }
} /*namespace xo*/

/* end init_object2.cpp */
