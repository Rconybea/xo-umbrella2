/** @file websock_reflect.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "websock_reflect.hpp"
#include "Webserver.hpp"
#include "WebsocketSink.hpp"
#include "WsSessionRouter.hpp"
#include "DynamicEndpoint.hpp"
#include "UrlRouter.hpp"
#include <xo/webutil/StreamReceiver.hpp>
#include <xo/webutil/StreamEndpointDescr.hpp>

namespace xo {
    namespace web {
        void
        websock_reflect_types(reflect::TypeDescrTable * table)
        {
            /* enums first: the structs below hold them */
            RunstateUtil::reflect_self(table);
            reflect_endpoint_kind(table);

            /* interfaces whose pointers should reach the actual type */
            StreamReceiver::reflect_self(table);
            /* value types the structs below hold */
            reflect_callback_id(table);

            WebserverConfig::reflect_self(table);
            Webserver::reflect_self(table);
            WebsocketSink::reflect_self(table);
            WsSessionRouter::reflect_self(table);
            DynamicEndpoint::reflect_self(table);
            UrlRouter::reflect_self(table);
        } /*websock_reflect_types*/
    } /*namespace web*/
} /*namespace xo*/

/* end websock_reflect.cpp */
