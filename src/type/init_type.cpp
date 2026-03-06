/** @file init_type.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "init_type.hpp"
#include "type_register_facets.hpp"
#include "type_register_types.hpp"
#include <xo/alloc2/CollectorTypeRegistry.hpp>
#include <xo/alloc2/init_alloc2.hpp>

namespace xo {
    using xo::scm::type_register_facets;
    using xo::scm::type_register_types;
    using xo::mm::CollectorTypeRegistry;

    void
    InitSubsys<S_type_tag>::init()
        {
            type_register_facets();

            CollectorTypeRegistry::instance().register_types(&type_register_types);
        }

    InitEvidence
    InitSubsys<S_type_tag>::require()
        {
            InitEvidence retval;

            /* direct subsystem deps for xo-type/ */
            retval ^= InitSubsys<S_alloc2_tag>::require();

            /* xo-type/'s own initialization code */
            retval ^= Subsystem::provide<S_type_tag>("type", &init);

            return retval;
        }
} /*namespace xo*/

/* end init_type.cpp */
