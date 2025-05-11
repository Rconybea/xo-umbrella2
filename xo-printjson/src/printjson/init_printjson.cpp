/* file init_printjson.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "init_printjson.hpp"
#include "PrintJson.hpp"
#include "xo/reflect/init_reflect.hpp"
#include "xo/subsys/Subsystem.hpp"

namespace xo {
    using xo::json::PrintJsonSingleton;

    void
    InitSubsys<S_printjson_tag>::init()
    {
        /* create singleton */
        PrintJsonSingleton::instance();
    } /*init*/

    InitEvidence
    InitSubsys<S_printjson_tag>::require()
    {
        InitEvidence retval;

        /* subsystem dependencies for printjson/ */
        retval ^= InitSubsys<S_reflect_tag>::require();

        /* printjson/'s own initialization code */
        retval ^= Subsystem::provide<S_printjson_tag>("printjson", &init);

        return retval;
    } /*require*/
} /*namespace xo*/

/* end init_printjson.cpp */
