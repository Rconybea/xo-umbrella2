/* file init_reactor.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "init_reactor.hpp"
#include "xo/reflect/init_reflect.hpp"

namespace xo {
    void
    InitSubsys<S_reactor_tag>::init()
    {
        /* TODO: reflect reactor types */
    } /*init*/

    InitEvidence
    InitSubsys<S_reactor_tag>::require()
    {
        InitEvidence retval;

        /* subsystem dependencies for reactor/ */
        retval ^= InitSubsys<S_reflect_tag>::require();

        /* reactor/'s own initialization code */
        retval ^= Subsystem::provide<S_reactor_tag>("reactor", &init);

        return retval;
    } /*require*/
} /*namespace xo*/

/* end init_reactor.cpp */
