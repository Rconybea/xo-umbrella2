/** @file webserver_json.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  INTERNAL to xo-websock (not installed): json printers for types private
 *  to one translation unit, defined where those types are visible -- a
 *  session's router (WsSessionRouter.cpp), a sink by its concrete type
 *  (WebsocketSink.cpp).  Called
 *  from provide_websock_json_printers.  The server, its session record,
 *  sender and subscriptions print from reflection, with no printer
 *  (.xo-backlog/xo-websock/issues/16)
 **/

#pragma once

#include <xo/printjson/PrintJson.hpp>

namespace xo {
    namespace web {
        void provide_router_json_printers(json::PrintJson * pjson);
        void provide_websocket_sink_json_printers(json::PrintJson * pjson);
    } /*namespace web*/
} /*namespace xo*/

/* end webserver_json.hpp */
