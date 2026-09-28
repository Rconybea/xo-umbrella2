/** @file UrlRouter.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  UrlRouter's matching: which registered endpoint serves an incoming uri.
 *  The matching was WebserverImpl's lookup_pattern, untested until it moved
 *  here (.xo-backlog/xo-websock/issues/07).  Endpoints are identified by
 *  stem(): stems are unique within one map.
 *
 *  Expectations are OBSERVED, never predicted.
 **/

#include "xo/websock/UrlRouter.hpp"
#include "xo/websock/DynamicEndpoint.hpp"
#include <catch2/catch.hpp>
#include <stdexcept>
#include <string>

namespace xo {
    using xo::web::UrlRouter;
    using xo::web::DynamicEndpoint;
    using xo::web::HttpEndpointDescr;
    using xo::web::StreamEndpointDescr;
    using xo::web::EndpointKind;
    using xo::web::endpoint_kind_descr;
    using xo::web::WebsocketSink;
    using xo::web::ContentType;
    using xo::web::HttpRequest;
    using xo::web::HttpResponse;
    using xo::web::HttpStatus;
    using xo::fn::CallbackId;

    namespace ut {
        namespace {
            /** http endpoint on @p pattern, answering "<label>:<uri>" **/
            HttpEndpointDescr http_descr(std::string pattern, std::string label) {
                return HttpEndpointDescr(std::move(pattern),
                                         [label](HttpRequest const & req)
                                             {
                                                 return HttpResponse::text(label + ":"
                                                                           + std::string(req.uri()));
                                             });
            }

            /** stream endpoint on @p pattern; subscribe/unsubscribe do nothing **/
            StreamEndpointDescr stream_descr(std::string pattern) {
                return StreamEndpointDescr(std::move(pattern),
                                           [](rp<WebsocketSink> const &) { return CallbackId(1); },
                                           [](CallbackId) {});
            }

            /** stem of the http endpoint serving @p uri; "" if none **/
            std::string http_stem(UrlRouter const & router, std::string const & uri) {
                rp<DynamicEndpoint> ep = router.find_http(uri);

                return ep ? ep->stem() : std::string();
            }

            std::string stream_stem(UrlRouter const & router, std::string const & uri) {
                rp<DynamicEndpoint> ep = router.find_stream(uri);

                return ep ? ep->stem() : std::string();
            }
        }

        TEST_CASE("url-router-empty-finds-nothing", "[websock][UrlRouter]")
        {
            UrlRouter router;

            REQUIRE(!router.find_http("/anything"));
            REQUIRE(!router.find_stream("/anything"));
            REQUIRE(!router.find_http(""));
        }

        TEST_CASE("url-router-literal-pattern-matches-whole-uri", "[websock][UrlRouter]")
        {
            UrlRouter router;
            router.register_http(http_descr("/status", "status"));

            REQUIRE(http_stem(router, "/status") == "/status");
            /* the empty uri matches nothing, even with endpoints present */
            REQUIRE(http_stem(router, "") == "");
            REQUIRE(http_stem(router, "/other") == "");
        }

        TEST_CASE("url-router-var-pattern-is-stored-under-its-stem", "[websock][UrlRouter]")
        {
            UrlRouter router;
            router.register_http(http_descr("/fw/${a}/detail", "fw"));

            rp<DynamicEndpoint> ep = router.find_http("/fw/q7/detail");

            REQUIRE(ep);
            REQUIRE(ep->stem() == "/fw/");

            REQUIRE(ep->http_response("/fw/q7/detail").body() == "fw:/fw/q7/detail");
        }

        TEST_CASE("url-router-longest-stem-wins", "[websock][UrlRouter]")
        {
            UrlRouter router;
            router.register_http(http_descr("/fw/${x}", "short"));
            router.register_http(http_descr("/fw/sub/${y}", "long"));

            REQUIRE(http_stem(router, "/fw/sub/7") == "/fw/sub/");
            REQUIRE(http_stem(router, "/fw/9") == "/fw/");
            /* registration order does not matter */
            UrlRouter router2;
            router2.register_http(http_descr("/fw/sub/${y}", "long"));
            router2.register_http(http_descr("/fw/${x}", "short"));

            REQUIRE(http_stem(router2, "/fw/sub/7") == "/fw/sub/");
            REQUIRE(http_stem(router2, "/fw/9") == "/fw/");
        }

        TEST_CASE("url-router-slash-terminated-stem-beats-a-longer-bare-prefix",
                  "[websock][UrlRouter]")
        {
            /* NOT strictly longest-prefix: after the whole uri, every prefix
             * ending in '/' is tried before any that does not.  Pinned as the
             * matching behaves today.
             */
            UrlRouter router;
            router.register_http(http_descr("/x/", "slash"));
            router.register_http(http_descr("/x/ab", "bare"));

            REQUIRE(http_stem(router, "/x/abc") == "/x/");
            /* the whole uri still comes first */
            REQUIRE(http_stem(router, "/x/ab") == "/x/ab");
        }

        TEST_CASE("url-router-bare-prefix-matches-when-no-slash-stem-does",
                  "[websock][UrlRouter]")
        {
            UrlRouter router;
            router.register_http(http_descr("/fwx", "bare"));

            REQUIRE(http_stem(router, "/fwxyz") == "/fwx");
            REQUIRE(http_stem(router, "/fwx/more") == "/fwx");
        }

        TEST_CASE("url-router-keeps-http-and-stream-apart", "[websock][UrlRouter]")
        {
            UrlRouter router;
            router.register_stream(stream_descr("/only-stream/${s}"));

            REQUIRE(stream_stem(router, "/only-stream/1") == "/only-stream/");
            REQUIRE(!router.find_http("/only-stream/1"));

            /* one stem, one endpoint in each map */
            router.register_http(http_descr("/both/${a}", "http"));
            router.register_stream(stream_descr("/both/${a}"));

            rp<DynamicEndpoint> h = router.find_http("/both/1");
            rp<DynamicEndpoint> s = router.find_stream("/both/1");

            REQUIRE(h);
            REQUIRE(s);
            REQUIRE(h.get() != s.get());
            REQUIRE(h->kind() == EndpointKind::http);
            REQUIRE(s->kind() == EndpointKind::stream);
        }

        TEST_CASE("url-router-rejects-a-duplicate-stem", "[websock][UrlRouter]")
        {
            /* was a silent replace until issue 07 */
            UrlRouter router;
            router.register_http(http_descr("/r/${a}", "old"));

            /* same pattern, and a different pattern with the same stem */
            REQUIRE_THROWS_AS(router.register_http(http_descr("/r/${a}", "new")),
                              std::runtime_error);
            REQUIRE_THROWS_AS(router.register_http(http_descr("/r/${b}/detail", "new")),
                              std::runtime_error);

            /* the original is untouched */
            REQUIRE(router.find_http("/r/1")->http_response("/r/1").body() == "old:/r/1");

            /* likewise for streams; and the http stem does not block it */
            router.register_stream(stream_descr("/r/${a}"));
            REQUIRE_THROWS_AS(router.register_stream(stream_descr("/r/${z}")),
                              std::runtime_error);
        }

        TEST_CASE("url-router-unregister-then-register-replaces", "[websock][UrlRouter]")
        {
            UrlRouter router;
            router.register_http(http_descr("/r/${a}", "old"));

            rp<DynamicEndpoint> old_ep = router.find_http("/r/1");

            REQUIRE(router.unregister_http("/r/${a}"));
            REQUIRE(!router.find_http("/r/1"));

            router.register_http(http_descr("/r/${b}", "new"));

            rp<DynamicEndpoint> new_ep = router.find_http("/r/1");

            REQUIRE(new_ep);
            REQUIRE(new_ep.get() != old_ep.get());

            /* the router dropped the old endpoint; our rp keeps it usable */
            REQUIRE(old_ep->http_response("/r/1").body() == "old:/r/1");
        }

        TEST_CASE("url-router-unregister-needs-the-exact-pattern", "[websock][UrlRouter]")
        {
            UrlRouter router;
            router.register_http(http_descr("/r/${a}", "http"));
            router.register_stream(stream_descr("/r/${a}"));

            /* nothing there */
            REQUIRE(!router.unregister_http("/nope"));
            /* same stem, different pattern: not the registered endpoint */
            REQUIRE(!router.unregister_http("/r/${b}"));
            REQUIRE(router.find_http("/r/1"));

            /* http and stream are unregistered separately */
            REQUIRE(router.unregister_http("/r/${a}"));
            REQUIRE(!router.find_http("/r/1"));
            REQUIRE(router.find_stream("/r/1"));

            /* a second unregister finds nothing */
            REQUIRE(!router.unregister_http("/r/${a}"));

            REQUIRE(router.unregister_stream("/r/${a}"));
            REQUIRE(!router.find_stream("/r/1"));
        }

        TEST_CASE("url-router-visits-its-endpoints", "[websock][UrlRouter]")
        {
            /* (kind, stem, pattern) of each endpoint visited, in visit order */
            struct Seen { EndpointKind kind; std::string stem; std::string pattern; };
            auto visit = [](UrlRouter const & router) {
                std::vector<Seen> v;
                router.visit_endpoints([&v](DynamicEndpoint const & ep) {
                        v.push_back(Seen{ep.kind(), ep.stem(), ep.uri_pattern()});
                    });
                return v;
            };

            UrlRouter router;

            REQUIRE(visit(router).empty());

            /* registered out of order, one stem in both maps */
            router.register_stream(stream_descr("/zz/${a}"));
            router.register_http(http_descr("/status", "s"));
            router.register_stream(stream_descr("/aa"));
            router.register_http(http_descr("/fw/${id}", "f"));
            router.register_stream(stream_descr("/status"));

            std::vector<Seen> v = visit(router);

            /* http then stream, each by stem */
            REQUIRE(v.size() == 5);

            REQUIRE(v[0].kind == EndpointKind::http);
            REQUIRE(v[0].stem == "/fw/");
            REQUIRE(v[0].pattern == "/fw/${id}");

            REQUIRE(v[1].kind == EndpointKind::http);
            REQUIRE(v[1].stem == "/status");

            REQUIRE(v[2].kind == EndpointKind::stream);
            REQUIRE(v[2].stem == "/aa");
            REQUIRE(v[3].kind == EndpointKind::stream);
            REQUIRE(v[3].stem == "/status");
            REQUIRE(v[4].kind == EndpointKind::stream);
            REQUIRE(v[4].stem == "/zz/");
            REQUIRE(v[4].pattern == "/zz/${a}");

            /* an unregistered endpoint is no longer visited */
            REQUIRE(router.unregister_stream("/status"));

            v = visit(router);
            REQUIRE(v.size() == 4);
            REQUIRE(v[1].kind == EndpointKind::http);
            REQUIRE(v[1].stem == "/status");
            REQUIRE(v[3].stem == "/zz/");

            REQUIRE(std::string(endpoint_kind_descr(EndpointKind::http)) == "http");
            REQUIRE(std::string(endpoint_kind_descr(EndpointKind::stream)) == "stream");
        }

        TEST_CASE("url-router-visit-sees-the-endpoint-itself", "[websock][UrlRouter]")
        {
            /* the object the router holds, not a copy -- and no rp<> taken
             * by the visit, so its refcount is exactly the router's hold
             */
            UrlRouter router;
            router.register_stream(stream_descr("/s/${x}"));

            DynamicEndpoint const * seen = nullptr;
            std::uint32_t seen_refcount = 0;

            router.visit_endpoints([&](DynamicEndpoint const & ep) {
                    seen = &ep;
                    seen_refcount = ep.reference_counter();
                });

            REQUIRE(seen_refcount == 1);

            rp<DynamicEndpoint> found = router.find_stream("/s/1");
            REQUIRE(found.get() == seen);
            /* find_stream's rp<> is a second hold */
            REQUIRE(found->reference_counter() == 2);
        }

        namespace {
            /** http endpoint on @p pattern answering, as text, each variable
             *  as name=value, one per line
             **/
            rp<DynamicEndpoint> vars_endpoint(std::string pattern) {
                return DynamicEndpoint::make_http
                    (std::move(pattern),
                     [](HttpRequest const & req) {
                         std::string out;
                         for (char const * v : {"a", "b", "path"}) {
                             if (!req.var(v).empty())
                                 out += std::string(v) + "=" + std::string(req.var(v)) + "\n";
                         }
                         return HttpResponse::text(out);
                     });
            }
        }

        TEST_CASE("http-endpoint-var-matches-one-segment", "[websock][DynamicEndpoint]")
        {
            auto ep = vars_endpoint("/fw/${a}/detail");

            /* any segment -- not only alphanumeric */
            REQUIRE(ep->http_response("/fw/q7/detail").body() == "a=q7\n");
            REQUIRE(ep->http_response("/fw/my-file_1.hpp/detail").body() == "a=my-file_1.hpp\n");

            /* not two segments, not none */
            REQUIRE(ep->http_response("/fw/q/7/detail").status() == HttpStatus::not_found());
            REQUIRE(ep->http_response("/fw//detail").status() == HttpStatus::not_found());
        }

        TEST_CASE("http-endpoint-rest-var-matches-the-rest", "[websock][DynamicEndpoint]")
        {
            auto ep = vars_endpoint("/src/${path...}");

            REQUIRE(ep->http_response("/src/a.hpp").body() == "path=a.hpp\n");
            REQUIRE(ep->http_response("/src/xo-foo/include/xo/foo/b.hpp").body()
                    == "path=xo-foo/include/xo/foo/b.hpp\n");
            REQUIRE(ep->http_response("/src/").status() == HttpStatus::not_found());

            /* with a segment variable before it */
            auto ep2 = vars_endpoint("/t/${a}/${path...}");
            REQUIRE(ep2->http_response("/t/x/y/z").body() == "a=x\npath=y/z\n");
        }

        TEST_CASE("http-endpoint-rest-var-must-be-last", "[websock][DynamicEndpoint]")
        {
            REQUIRE_THROWS_AS(vars_endpoint("/src/${path...}/more"), std::invalid_argument);
        }

        TEST_CASE("http-endpoint-fixed-text-is-literal", "[websock][DynamicEndpoint]")
        {
            /* '.' in the pattern is a dot, not any character */
            auto ep = vars_endpoint("/v1.0/${a}");

            REQUIRE(ep->http_response("/v1.0/x").body() == "a=x\n");
            REQUIRE(ep->http_response("/v1x0/x").status() == HttpStatus::not_found());
        }

        TEST_CASE("http-endpoint-no-match-is-not-found", "[websock][DynamicEndpoint]")
        {
            /* found by its stem, but the uri does not match the pattern: the
             * handler is not called
             */
            bool called = false;
            auto ep = DynamicEndpoint::make_http("/hello/${a}",
                                                 [&called](HttpRequest const &) {
                                                     called = true;
                                                     return HttpResponse::text("x");
                                                 });

            HttpResponse r = ep->http_response("/hello/a/b");

            REQUIRE(r.status() == HttpStatus::not_found());
            REQUIRE(r.content_type() == ContentType::html);
            REQUIRE(!called);
        }

        TEST_CASE("http-endpoint-response-comes-through", "[websock][DynamicEndpoint]")
        {
            auto ep = DynamicEndpoint::make_http("/t",
                                                 [](HttpRequest const &) {
                                                     return HttpResponse(HttpStatus(418),
                                                                         ContentType::text,
                                                                         "teapot");
                                                 });

            HttpResponse r = ep->http_response("/t");

            REQUIRE(r.status().code() == 418);
            REQUIRE(r.content_type() == ContentType::text);
            REQUIRE(r.body() == "teapot");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end UrlRouter.test.cpp */
