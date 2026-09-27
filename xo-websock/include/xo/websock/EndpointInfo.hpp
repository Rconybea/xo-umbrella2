/** @file EndpointInfo.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <string>

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

        /** @brief a registered endpoint, as a plain value: what a listing
         *  (UrlRouter::endpoints, Webserver::endpoints) reports.
         *
         *  A copy, taken under the router's lock; holds no reference to the
         *  endpoint itself.
         **/
        struct EndpointInfo {
            EndpointKind kind_ = EndpointKind::http;
            /* key in the router's map: longest literal prefix of the pattern */
            std::string stem_;
            /* as registered, e.g. "/fw/${id}" */
            std::string uri_pattern_;
        };
    } /*namespace web*/
} /*namespace xo*/

/* end EndpointInfo.hpp */
