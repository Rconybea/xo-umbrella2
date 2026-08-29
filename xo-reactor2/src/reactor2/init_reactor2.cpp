/* file init_reactor2.cpp
 *
 * author: Roland Conybeare, Aug 2026
 */

#include "init_reactor2.hpp"
#include "setup_reactor2.hpp"
#include <xo/stringtable2/init_stringtable2.hpp>
#include <xo/reflect/init_reflect.hpp>
#include <xo/subsys/Subsystem.hpp>

namespace xo {
    void
    InitSubsys<S_reactor2_tag>::init()
    {
        using xo::process::SetupReactor2;

        /* placeholder -- expecting there to be non-trivial content soon */

        SetupReactor2::register_facets();

    } /*init*/

    InitEvidence
    InitSubsys<S_reactor2_tag>::require()
    {
        InitEvidence retval;

        /* direct subsystem deps for xo-reactor2/ */
        retval ^= InitSubsys<S_stringtable2_tag>::require();

        /* direct subsystem deps for xo-reactor2/ */
        retval ^= InitSubsys<S_reflect_tag>::require();

        /* xo-reactor2/'s own initialization code */
        retval ^= Subsystem::provide<S_reactor2_tag>("reactor2", &init);

        return retval;
    } /*require*/
} /*namespace xo*/

/* end init_reactor2.cpp */
