/* file init_process.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "init_process.hpp"
#include "xo/printjson/init_printjson.hpp"

#include "UpxEvent.hpp"
#include "xo/subsys/Subsystem.hpp"

namespace xo {
    using xo::process::UpxEvent;

    void
    InitSubsys<S_process_tag>::init()
    {
        UpxEvent::reflect_self();
    } /*init*/

    InitEvidence
    InitSubsys<S_process_tag>::require()
    {
        InitEvidence retval;

        /* direct subsystem dependencies for process/
         *
         * UpxEventStore --uses-> printjson  (via reactor/EventStore.hpp)
         */
        retval ^= InitSubsys<S_printjson_tag>::require();

        /* process/'s own initialization code */
        retval ^= Subsystem::provide<S_process_tag>("process", &init);

        return retval;
    } /*require*/

} /*namespace xo*/

/* end init_process.cpp */
