/** @file UrlRouter.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "UrlRouter.hpp"
#include "DynamicEndpoint.hpp"
#include "webserver_json.hpp"
#include <xo/printjson/JsonPrinter.hpp>
#include <xo/printjson/JsonMembers.hpp>
#include <xo/printjson/type_keys.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>      /* xtag(..) */
#include <xo/reflect/StructReflector.hpp>
#include <algorithm>
#include <stdexcept>

namespace xo {
    using xo::reflect::StructReflector;
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
                                                       descr.handler());

            this->insert_in(std::move(endpoint), "http", &this->http_map_);
        } /*register_http*/

        void
        UrlRouter::register_stream(StreamEndpointDescr const & descr)
        {
            auto endpoint = DynamicEndpoint::make_stream(descr.uri_pattern(),
                                                         descr.subscribe_fn(),
                                                         descr.unsubscribe_fn(),
                                                         descr.receiver());

            this->insert_in(std::move(endpoint), "stream", &this->stream_map_);
        } /*register_stream*/

        rp<DynamicEndpoint>
        UrlRouter::unregister_http(std::string const & uri_pattern)
        {
            return this->erase_in(uri_pattern, &this->http_map_);
        }

        rp<DynamicEndpoint>
        UrlRouter::unregister_stream(std::string const & uri_pattern)
        {
            return this->erase_in(uri_pattern, &this->stream_map_);
        }

        void
        UrlRouter::visit_endpoints(EndpointVisitor const & fn) const
        {
            std::lock_guard<std::mutex> lock(this->mutex_);

            /* the maps are unordered: sort, so a visit order is stable */
            std::vector<DynamicEndpoint const *> ep_v;

            ep_v.reserve(this->http_map_.size() + this->stream_map_.size());

            for (auto const * ep_map : {&this->http_map_, &this->stream_map_}) {
                for (auto const & ix : *ep_map)
                    ep_v.push_back(ix.second.get());
            }

            std::sort(ep_v.begin(), ep_v.end(),
                      [](DynamicEndpoint const * x, DynamicEndpoint const * y)
                          {
                              if (x->kind() != y->kind())
                                  return x->kind() < y->kind();
                              return x->stem() < y->stem();
                          });

            for (DynamicEndpoint const * ep : ep_v)
                fn(*ep);
        } /*visit_endpoints*/

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

        void
        UrlRouter::insert_in(rp<DynamicEndpoint> endpoint,
                             char const * kind,
                             EndpointMap * p_ep_map)
        {
            std::string stem = endpoint->stem();

            std::lock_guard<std::mutex> lock(this->mutex_);

            auto ix = p_ep_map->find(stem);

            if (ix != p_ep_map->end()) {
                /* was a silent replace until issue 07 */
                throw std::runtime_error
                    (std::string("UrlRouter: ") + kind + " endpoint ["
                     + endpoint->uri_pattern()
                     + "] has the same stem [" + stem
                     + "] as registered endpoint [" + ix->second->uri_pattern()
                     + "]; unregister that first");
            }

            p_ep_map->emplace(std::move(stem), std::move(endpoint));
        } /*insert_in*/

        rp<DynamicEndpoint>
        UrlRouter::erase_in(std::string const & uri_pattern,
                            EndpointMap * p_ep_map)
        {
            /* returned, so released after the lock: dropping what may be the
             * last reference runs the endpoint's dtor, and with it the dtors
             * of whatever its functions capture
             */
            rp<DynamicEndpoint> removed;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                auto ix = p_ep_map->find(EndpointUtil::stem(uri_pattern));

                if ((ix == p_ep_map->end())
                    || (ix->second->uri_pattern() != uri_pattern))
                {
                    return nullptr;
                }

                removed = std::move(ix->second);
                p_ep_map->erase(ix);
            }

            return removed;
        } /*erase_in*/
        void
        UrlRouter::reflect_self(reflect::TypeDescrTable * /*table*/)
        {
            /* no members yet: a member is added as a printer opts in to
             * show it (.xo-backlog/xo-websock/issues/13)
             */
            StructReflector<UrlRouter> sr;
        } /*reflect_self*/
        /** @brief the url router: its chosen C++ members
         *  (.xo-backlog/xo-websock/issues/13).  Not in the anonymous
         *  namespace: UrlRouter's header befriends it by name.
         *
         *  Printed with an id, so a ref to it (a session router's
         *  url_router_) can be joined to it.  Its endpoints are printed in
         *  full in the server's endpoint list: refs here, keyed by stem,
         *  sorted -- an unordered_map has no stable order
         **/
        class JsonPrinter_UrlRouter : public json::JsonPrinter {
        public:
            JsonPrinter_UrlRouter(json::PrintJson const * pjson) : JsonPrinter(pjson) {}

            void print_json(reflect::TaggedPtr tp, std::ostream * p_os) const override {
                using xo::pp::quot;
                using Entries = std::vector<std::pair<std::string, void const *>>;

                UrlRouter const * r = this->check_recover_native<UrlRouter>(tp, p_os);

                if (!r)
                    return;

                Entries http_v;
                Entries stream_v;
                {
                    std::lock_guard<std::mutex> lock(r->mutex_);

                    for (auto const & ix : r->http_map_)
                        http_v.emplace_back(ix.first, ix.second.get());
                    for (auto const & ix : r->stream_map_)
                        stream_v.emplace_back(ix.first, ix.second.get());
                }
                std::sort(http_v.begin(), http_v.end());
                std::sort(stream_v.begin(), stream_v.end());

                *p_os << "{" << quot("_name_") << ": " << quot("UrlRouter")
                      << ", " << json::type_keys(tp.td())
                      << ", " << quot("id") << ": " << quot(json_id(r));

                json::JsonMembers mem(this->pjson(), p_os);
                mem.member_ref_map<EndpointMap>("http_map_", http_v)
                    .member_ref_map<EndpointMap>("stream_map_", stream_v);
                mem.end();

                *p_os << "}";
            }
        };

        void
        provide_url_router_json_printers(json::PrintJson * pjson)
        {
            pjson->provide_printer(reflect::Reflect::require<UrlRouter>(),
                                   std::make_unique<JsonPrinter_UrlRouter>(pjson));
        }
    } /*namespace web*/
} /*namespace xo*/

/* end UrlRouter.cpp */
