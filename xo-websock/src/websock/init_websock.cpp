/** @file init_websock.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "init_websock.hpp"
#include <xo/printjson/init_printjson.hpp>
#include <xo/subsys/Subsystem.hpp>

namespace xo {
    void
    InitSubsys<S_websock_tag>::init()
    {
        /* nothing yet: websock/'s setup lives in WebsockAppcx */
    } /*init*/

    InitEvidence
    InitSubsys<S_websock_tag>::require()
    {
        InitEvidence retval;

        /* subsystem dependencies for websock/ */
        retval ^= InitSubsys<S_printjson_tag>::require();

        /* websock/'s own initialization code */
        retval ^= Subsystem::provide<S_websock_tag>("websock", &init);

        return retval;
    } /*require*/
} /*namespace xo*/

/* end init_websock.cpp */
