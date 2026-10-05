/** @file webserver_json.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  INTERNAL to xo-websock (not installed): json printers for types private
 *  to one translation unit, defined where those types are visible --
 *  the server (WebserverImpl), its session record and sender
 *  (Webserver.cpp), a router's subscription
 *  (WsSessionRouter.cpp).  Called from provide_websock_json_printers.
 **/

#pragma once

#include <xo/printjson/PrintJson.hpp>

namespace xo {
    namespace web {
        void provide_webserver_json_printers(json::PrintJson * pjson);
        void provide_router_json_printers(json::PrintJson * pjson);
        void provide_url_router_json_printers(json::PrintJson * pjson);
    } /*namespace web*/
} /*namespace xo*/

/* end webserver_json.hpp */
