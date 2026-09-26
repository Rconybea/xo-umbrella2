/** @file UrlRouter.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "UrlRouter.hpp"
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>      /* xtag(..) */

namespace xo {
    namespace web {
        using xo::pp::scope;
        using xo::pp::xtag;

        namespace {
            using EndpointMap = std::unordered_map<std::string,
                                                   rp<DynamicEndpoint>>;

            DynamicEndpoint *
            lookup_stem(std::string const & stem,
                        EndpointMap const & ep_map)
            {
                scope log(XO_DEBUG_(true /*debug_flag*/),
                          xtag("stem", stem));

                auto ix = ep_map.find(stem);

                if (ix != ep_map.end())
                    return ix->second.get();
                else
                    return nullptr;
            } /*lookup_stem*/

            DynamicEndpoint *
            lookup_pattern(std::string const & incoming_uri,
                           EndpointMap const & ep_map)
            {
                if (incoming_uri.empty())
                    return nullptr;

                /* find longest prefix of incoming_uri that is a stem in ep_map.
                 *
                 * 1. try the whole uri
                 * 2. try successively shorter prefixes of uri that end in '/'
                 * 3. try successively shorter prefixes of uri that do not end in '/'
                 */

                /* 1. try the whole uri */
                DynamicEndpoint * endpoint = nullptr;

                endpoint = lookup_stem(incoming_uri, ep_map);

                if (!endpoint) {
                    /* 2. try successively shorter prefixes of uri that end in '/'.
                     *    we already checked for the whole uri,  so look for a match
                     *    at or before the 2nd-last character
                     */
                    if (incoming_uri.size() >= 2) {
                        std::string::size_type p = incoming_uri.size() - 1;

                        while (!endpoint) {
                            p = incoming_uri.find_last_of('/', p-1);

                            if (p == std::string::npos)
                                break;

                            endpoint
                                = lookup_stem(incoming_uri.substr(0, p+1), ep_map);

                            if (p == 0)
                                break;
                        }
                    }
                }

                if (!endpoint) {
                    /* 3. try successively shorter prefixes of uri that don't end in '/'.
                     */
                    if (incoming_uri.size() >= 2) {
                        std::string::size_type p = incoming_uri.size() - 2;

                        while (!endpoint) {
                            if (incoming_uri[p] == '/') {
                                /* all stems ending in '/' have already been excluded */
                                ;
                            } else {
                                endpoint
                                    = lookup_stem(incoming_uri.substr(0, p+1), ep_map);
                            }

                            if (p == 0)
                                break;

                            --p;
                        }
                    }
                }

                return endpoint;
            } /*lookup_pattern*/
        } /*namespace*/

        void
        UrlRouter::register_http(HttpEndpointDescr const & descr)
        {
            auto endpoint = DynamicEndpoint::make_http(descr.uri_pattern(),
                                                       descr.endpoint_fn());

            std::lock_guard<std::mutex> lock(this->mutex_);

            this->http_map_[endpoint->stem()] = std::move(endpoint);
        } /*register_http*/

        void
        UrlRouter::register_stream(StreamEndpointDescr const & descr)
        {
            auto endpoint = DynamicEndpoint::make_stream(descr.uri_pattern(),
                                                         descr.subscribe_fn(),
                                                         descr.unsubscribe_fn(),
                                                         descr.receive_fn());

            std::lock_guard<std::mutex> lock(this->mutex_);

            this->stream_map_[endpoint->stem()] = std::move(endpoint);
        } /*register_stream*/

        rp<DynamicEndpoint>
        UrlRouter::find_http(std::string const & uri) const
        {
            return this->find_in(uri, this->http_map_);
        }

        rp<DynamicEndpoint>
        UrlRouter::find_stream(std::string const & stream_name) const
        {
            return this->find_in(stream_name, this->stream_map_);
        }

        rp<DynamicEndpoint>
        UrlRouter::find_in(std::string const & uri,
                           EndpointMap const & ep_map) const
        {
            std::lock_guard<std::mutex> lock(this->mutex_);

            /* rp<> taken while locked: a concurrent register may drop the
             * map's reference as soon as the lock is released
             */
            return rp<DynamicEndpoint>(lookup_pattern(uri, ep_map));
        } /*find_in*/
    } /*namespace web*/
} /*namespace xo*/

/* end UrlRouter.cpp */
