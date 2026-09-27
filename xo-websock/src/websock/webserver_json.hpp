/** @file webserver_json.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  INTERNAL to xo-websock (not installed): json printers for types private
 *  to one translation unit, defined where those types are visible --
 *  the session record and sender (Webserver.cpp), a router's subscription
 *  (WsSessionRouter.cpp).  Called from provide_websock_json_printers.
 **/

#pragma once

#include <xo/printjson/PrintJson.hpp>
#include <sstream>
#include <string>

namespace xo {
    namespace web {
        void provide_webserver_json_printers(json::PrintJson * pjson);
        void provide_router_json_printers(json::PrintJson * pjson);

        /** an object's identity on the page: its address, as a json string.
         *  Unique within one snapshot; an address may be reused once its
         *  object is freed, so not across snapshots.  A ref to an object
         *  prints this same string, so the page can join them
         **/
        inline std::string json_id(void const * p) {
            std::ostringstream ss;
            ss << p;
            return ss.str();
        }
    } /*namespace web*/
} /*namespace xo*/

/* end webserver_json.hpp */
