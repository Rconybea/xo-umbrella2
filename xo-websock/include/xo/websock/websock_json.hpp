/** @file websock_json.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/printjson/PrintJson.hpp>

namespace xo {
    namespace web {
        /** install xo-websock's json printers into @p pjson.
         *
         *  So that PrintJson can print the webserver's own objects -- e.g. a
         *  reflected struct holding a @c Webserver* -- for introspection.
         *  Idempotent: PrintJson keeps the first printer installed for a type.
         *  WebsockAppcx calls this on its PrintJson (.xo-backlog/xo-websock/issues/11).
         *
         *  Printers installed:
         *  - Webserver: id, refcount, listen_port, state, endpoints, sessions
         *  - DynamicEndpoint: id, refcount, kind, stem, pattern, has_receive
         *
         *  See .xo-backlog/xo-websock/issues/10.
         **/
        void provide_websock_json_printers(json::PrintJson * pjson);
    } /*namespace web*/
} /*namespace xo*/

/* end websock_json.hpp */
