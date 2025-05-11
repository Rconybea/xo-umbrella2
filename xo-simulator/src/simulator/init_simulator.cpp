/* file init_simulator.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "init_simulator.hpp"
#include "xo/reactor/init_reactor.hpp"

namespace xo {
    void
    InitSubsys<S_simulator_tag>::init()
    {
    } /*init*/

    InitEvidence
    InitSubsys<S_simulator_tag>::require()
    {
        InitEvidence retval;

        /* subsystem dependencies for simulator/ */
        retval ^= InitSubsys<S_reactor_tag>::require();

        /* simulator/'s own initialization code */
        retval ^= XO_SUBSYSTEM_PROVIDE(simulator, &init);
        //retval ^= Subsystem::provide<S_simulator_tag>("simulator", &init);

        return retval;
    } /*require*/
} /*namespace xo*/

/* end init_simulator.cpp */
