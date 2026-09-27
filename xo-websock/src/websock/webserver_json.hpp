/** @file webserver_json.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  INTERNAL to xo-websock (not installed): json printers for types private
 *  to Webserver.cpp -- the session record and the session sender -- defined
 *  there, where those types are visible.  Called from
 *  provide_websock_json_printers.
 **/

#pragma once

#include <xo/printjson/PrintJson.hpp>

namespace xo {
    namespace web {
        void provide_webserver_json_printers(json::PrintJson * pjson);
    } /*namespace web*/
} /*namespace xo*/

/* end webserver_json.hpp */
