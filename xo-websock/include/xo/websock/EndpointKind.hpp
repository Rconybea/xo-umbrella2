/** @file EndpointKind.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <functional>

namespace xo {
    namespace web {
        /* which kind of endpoint a DynamicEndpoint is; fixed at construction
         * by make_http() / make_stream()
         */
        enum class EndpointKind {
            /* serves http requests: http_response() */
            http,
            /* serves websocket subscriptions: subscribe(), unsubscribe(),
             * and receive() if it has a receiver
             */
            stream,
        };

        /** "http" or "stream" **/
        inline char const * endpoint_kind_descr(EndpointKind x) {
            switch (x) {
            case EndpointKind::http:   return "http";
            case EndpointKind::stream: return "stream";
            }
            return "???";
        }

        class DynamicEndpoint;

        /** visits a registered endpoint, e.g. for introspection -- see
         *  UrlRouter::visit_endpoints, Webserver::visit_endpoints
         **/
        using EndpointVisitor = std::function<void (DynamicEndpoint const & endpoint)>;
    } /*namespace web*/
} /*namespace xo*/

/* end EndpointKind.hpp */
