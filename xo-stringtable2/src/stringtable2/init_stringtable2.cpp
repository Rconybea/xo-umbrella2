/** @file init_stringtable2.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "init_stringtable2.hpp"
#include "stringtable2_register_facets.hpp"
#include "stringtable2_register_types.hpp"
#include <xo/stringtable2/init_stringtable2.hpp>
//n#include <xo/printable2/init_printable2.hpp>
#include <xo/alloc2/CollectorTypeRegistry.hpp>
#include <xo/alloc2/init_alloc2.hpp>

namespace xo {
    using xo::scm::stringtable2_register_facets;
    using xo::scm::stringtable2_register_types;
    using xo::mm::CollectorTypeRegistry;

    void
    InitSubsys<S_stringtable2_tag>::init()
        {
            stringtable2_register_facets();

            CollectorTypeRegistry::instance().register_types(&stringtable2_register_types);
        }

    InitEvidence
    InitSubsys<S_stringtable2_tag>::require()
        {
            InitEvidence retval;

            /* direct subsystem deps for xo-stringtable2/ */
            retval ^= InitSubsys<S_alloc2_tag>::require();
            //retval ^= InitSubsys<S_printable_tag>::require();

            /* xo-expression2/'s own initialization code */
            retval ^= Subsystem::provide<S_stringtable2_tag>("stringtable2", &init);

            return retval;
        }
} /*namespace xo*/

/* end init_stringtable2.cpp */
