/** @file UrlRouter.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "DynamicEndpoint.hpp"
#include <xo/webutil/HttpEndpointDescr.hpp>
#include <xo/webutil/StreamEndpointDescr.hpp>
#include <xo/refcnt/Refcounted.hpp>
#include <mutex>
#include <string>
#include <unordered_map>

namespace xo {
    namespace web {
        /** @brief server-wide: which endpoint serves a uri or stream name.
         *
         *  Holds the webserver's registered endpoints, http and stream kept
         *  apart, and the matching that picks one for an incoming uri.
         *  Extracted from WebserverImpl; see .xo-backlog/xo-websock/issues/07.
         *
         *  Both maps are keyed by stem -- the longest literal prefix of an
         *  endpoint's uri pattern, see EndpointUtil::stem().  E.g. endpoint
         *  with pattern
         *    /fixed/stem/${a}/more
         *  is stored under
         *    /fixed/stem/
         *  An http endpoint and a stream endpoint may share a stem.
         *
         *  Registering a stem already present in the same map is an error;
         *  replacing an endpoint takes an explicit unregister first.
         *  "Same stem" is coarser than "same pattern": /fw/${a} and
         *  /fw/${a}/detail both have stem /fw/, so the second is rejected.
         *
         *  Unregistering drops only the router's reference.  Anyone else
         *  holding the endpoint -- e.g. a live subscription -- keeps it alive.
         *
         *  THREADING: all methods may be called from any thread.
         *  Registration typically runs on the application's thread, lookup on
         *  the webserver's service thread.
         **/
        class UrlRouter {
        public:
            /** register http endpoint described by @p descr.
             *  Throws std::runtime_error if an http endpoint with the same
             *  stem is already registered.
             **/
            void register_http(HttpEndpointDescr const & descr);

            /** register stream endpoint described by @p descr.
             *  Throws std::runtime_error if a stream endpoint with the same
             *  stem is already registered.
             **/
            void register_stream(StreamEndpointDescr const & descr);

            /** remove the http endpoint registered with exactly @p uri_pattern.
             *  Returns false, changing nothing, if there is none -- including
             *  when a DIFFERENT pattern with the same stem is registered.
             **/
            bool unregister_http(std::string const & uri_pattern);

            /** remove the stream endpoint registered with exactly
             *  @p uri_pattern.  Returns false, changing nothing, if there is
             *  none; see unregister_http().
             **/
            bool unregister_stream(std::string const & uri_pattern);

            /** http endpoint serving @p uri; null if none.  See find_in(). **/
            rp<DynamicEndpoint> find_http(std::string const & uri) const;

            /** stream endpoint serving @p stream_name; null if none.
             *  See find_in().
             **/
            rp<DynamicEndpoint> find_stream(std::string const & stream_name) const;

        private:
            using EndpointMap = std::unordered_map<std::string,
                                                   rp<DynamicEndpoint>>;

            /** find longest prefix of @p uri that is a stem in @p ep_map:
             *
             *  1. try the whole uri
             *  2. try successively shorter prefixes of uri that end in '/'
             *  3. try successively shorter prefixes of uri that do not end in '/'
             *
             *  Result copied out under the lock: caller uses the endpoint with
             *  the lock released, since endpoint functions may re-enter the
             *  server.
             **/
            rp<DynamicEndpoint> find_in(std::string const & uri,
                                        EndpointMap const & ep_map) const;

            /** insert @p endpoint into @p *p_ep_map; @p kind ("http" or
             *  "stream") names the map in the duplicate error
             **/
            void insert_in(rp<DynamicEndpoint> endpoint,
                           char const * kind,
                           EndpointMap * p_ep_map);

            /** remove the endpoint in @p *p_ep_map registered with exactly
             *  @p uri_pattern; false if none
             **/
            bool erase_in(std::string const & uri_pattern,
                          EndpointMap * p_ep_map);

        private:
            /* guards .http_map, .stream_map */
            mutable std::mutex mutex_;

            /* map :: stem -> http endpoint
             * use .register_http() to insert
             */
            EndpointMap http_map_;
            /* map :: stem -> stream endpoint
             * use .register_stream() to insert
             */
            EndpointMap stream_map_;
        }; /*UrlRouter*/
    } /*namespace web*/
} /*namespace xo*/

/* end UrlRouter.hpp */
