/** @file init_object2.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "init_object2.hpp"
#include "SetupObject2.hpp"
#include <xo/stringtable2/init_stringtable2.hpp>
#include <xo/alloc2/CollectorTypeRegistry.hpp>
#include <xo/alloc2/init_alloc2.hpp>

namespace xo {
    using xo::scm::SetupObject2;
    using xo::mm::CollectorTypeRegistry;

    void
    InitSubsys<S_object2_tag>::init()
        {
            SetupObject2::register_facets();

            CollectorTypeRegistry::instance().register_types(&SetupObject2::register_types);
        }

    InitEvidence
    InitSubsys<S_object2_tag>::require()
        {
            InitEvidence retval;

            /* direct subsystem deps for xo-object2/ */
            retval ^= InitSubsys<S_alloc2_tag>::require();
            retval ^= InitSubsys<S_stringtable2_tag>::require();

            /* xo-expression2/'s own initialization code */
            retval ^= Subsystem::provide<S_object2_tag>("object2", &init);

            return retval;
        }
} /*namespace xo*/

/* end init_object2.cpp */
