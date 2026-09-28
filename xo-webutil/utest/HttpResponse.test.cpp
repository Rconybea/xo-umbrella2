/** @file HttpResponse.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  xo::web::{HttpStatus, HttpRequest, HttpResponse}, and HttpEndpointDescr's
 *  handler.  See .xo-backlog/xo-websock/issues/12, step 4a.
 **/

#include "xo/webutil/HttpEndpointDescr.hpp"
#include "xo/webutil/HttpRequest.hpp"
#include "xo/webutil/HttpResponse.hpp"
#include "xo/webutil/HttpStatus.hpp"
#include <catch2/catch.hpp>
#include <string>
#include <type_traits>

namespace xo {
    using xo::web::Alist;
    using xo::web::ContentType;
    using xo::web::HttpEndpointDescr;
    using xo::web::HttpRequest;
    using xo::web::HttpResponse;
    using xo::web::HttpStatus;

    namespace ut {
        TEST_CASE("http-status", "[webutil][HttpStatus]")
        {
            STATIC_REQUIRE(HttpStatus::ok() == HttpStatus(200));
            STATIC_REQUIRE(HttpStatus::forbidden().code() == 403);
            STATIC_REQUIRE(HttpStatus::not_found().code() == 404);
            STATIC_REQUIRE(HttpStatus::internal_error().code() == 500);

            /* converts to int; made from an int only explicitly */
            int n = HttpStatus::not_found();
            REQUIRE(n == 404);
            STATIC_REQUIRE(std::is_convertible_v<HttpStatus, int>);
            STATIC_REQUIRE(!std::is_convertible_v<int, HttpStatus>);
        }

        TEST_CASE("http-request", "[webutil][HttpRequest]")
        {
            Alist vars;
            vars.push_back("path", "xo-foo/include/a.hpp");

            HttpRequest req("/src/xo-foo/include/a.hpp", vars);

            REQUIRE(req.uri() == "/src/xo-foo/include/a.hpp");
            REQUIRE(req.var("path") == "xo-foo/include/a.hpp");
            REQUIRE(req.var("absent") == "");
        }

        TEST_CASE("http-response-named-ctors", "[webutil][HttpResponse]")
        {
            auto j = HttpResponse::json("{}");
            REQUIRE(j.status() == HttpStatus::ok());
            REQUIRE(j.content_type() == ContentType::json);
            REQUIRE(j.body() == "{}");

            auto h = HttpResponse::html("<p>x</p>");
            REQUIRE(h.status() == HttpStatus::ok());
            REQUIRE(h.content_type() == ContentType::html);

            auto t = HttpResponse::text("x");
            REQUIRE(t.status() == HttpStatus::ok());
            REQUIRE(t.content_type() == ContentType::text);
        }

        TEST_CASE("http-response-public-ctor", "[webutil][HttpResponse]")
        {
            HttpResponse r(HttpStatus(418), ContentType::text, "teapot");

            REQUIRE(r.status().code() == 418);
            REQUIRE(r.content_type() == ContentType::text);
            REQUIRE(r.body() == "teapot");
        }

        TEST_CASE("http-response-errors-escape-their-message", "[webutil][HttpResponse]")
        {
            auto nf = HttpResponse::not_found("no <such> file & \"so on\"");
            REQUIRE(nf.status() == HttpStatus::not_found());
            REQUIRE(nf.content_type() == ContentType::html);
            REQUIRE(nf.body().find("no &lt;such&gt; file &amp; &quot;so on&quot;")
                    != std::string::npos);
            REQUIRE(nf.body().find("<such>") == std::string::npos);

            auto fb = HttpResponse::forbidden("outside the tree");
            REQUIRE(fb.status() == HttpStatus::forbidden());
            REQUIRE(fb.content_type() == ContentType::html);
            REQUIRE(fb.body().find("outside the tree") != std::string::npos);

            auto ie = HttpResponse::internal_error("boom");
            REQUIRE(ie.status() == HttpStatus::internal_error());
            REQUIRE(ie.content_type() == ContentType::html);
            REQUIRE(ie.body().find("boom") != std::string::npos);
        }

        TEST_CASE("content-type-str", "[webutil][HttpResponse]")
        {
            using xo::web::content_type_str;

            REQUIRE(std::string(content_type_str(ContentType::json)) == "application/json");
            REQUIRE(std::string(content_type_str(ContentType::html)) == "text/html; charset=utf-8");
            REQUIRE(std::string(content_type_str(ContentType::text)) == "text/plain; charset=utf-8");
        }

        TEST_CASE("http-endpoint-handler", "[webutil][HttpEndpointDescr]")
        {
            HttpEndpointDescr d("/r/${x}",
                                [](HttpRequest const & req) {
                                    return HttpResponse::html("<p>" + std::string(req.var("x"))
                                                              + "</p>");
                                });

            Alist vars;
            vars.push_back("x", "1");

            HttpResponse r = d.handler()(HttpRequest("/r/1", vars));

            REQUIRE(r.status() == HttpStatus::ok());
            REQUIRE(r.content_type() == ContentType::html);
            REQUIRE(r.body() == "<p>1</p>");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end HttpResponse.test.cpp */
