/** @file websock_reflect.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

namespace xo {
    namespace reflect { class TypeDescrTable; }

    namespace web {
        /** describe xo-websock's types to xo-reflect, in @p table.
         *
         *  Each non-atomic type in scope has a static reflect_self(table);
         *  a public class's also covers the implementation types defined in
         *  its .cpp (e.g. Webserver::reflect_self: WebserverImpl, ..).
         *  WebsockAppcx calls this, before installing json printers
         *  (.xo-backlog/xo-websock/issues/13).
         *
         *  @p table is not used yet: xo-reflect's registration uses its
         *  process-wide table.
         **/
        void websock_reflect_types(reflect::TypeDescrTable * table);
    } /*namespace web*/
} /*namespace xo*/

/* end websock_reflect.hpp */
