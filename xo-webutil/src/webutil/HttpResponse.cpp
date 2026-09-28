/** @file HttpResponse.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "HttpResponse.hpp"
#include <utility>

namespace xo {
    namespace web {
        char const *
        content_type_str(ContentType x)
        {
            switch (x) {
            case ContentType::json:
                return "application/json";
            case ContentType::html:
                return "text/html; charset=utf-8";
            case ContentType::text:
                return "text/plain; charset=utf-8";
            }

            return "application/octet-stream";
        }

        std::string
        html_escape(std::string_view s)
        {
            std::string retval;
            retval.reserve(s.size());

            for (char c : s) {
                switch (c) {
                case '&': retval += "&amp;"; break;
                case '<': retval += "&lt;"; break;
                case '>': retval += "&gt;"; break;
                case '"': retval += "&quot;"; break;
                default:  retval += c; break;
                }
            }

            return retval;
        }

        HttpResponse::HttpResponse(HttpStatus status, ContentType content_type, std::string body)
            : status_{status},
              content_type_{content_type},
              body_{std::move(body)}
        {}

        HttpResponse
        HttpResponse::json(std::string body)
        {
            return HttpResponse(HttpStatus::ok(), ContentType::json, std::move(body));
        }

        HttpResponse
        HttpResponse::html(std::string body)
        {
            return HttpResponse(HttpStatus::ok(), ContentType::html, std::move(body));
        }

        HttpResponse
        HttpResponse::text(std::string body)
        {
            return HttpResponse(HttpStatus::ok(), ContentType::text, std::move(body));
        }

        HttpResponse
        HttpResponse::not_found(std::string_view msg)
        {
            return error_page(HttpStatus::not_found(), "not found", msg);
        }

        HttpResponse
        HttpResponse::forbidden(std::string_view msg)
        {
            return error_page(HttpStatus::forbidden(), "forbidden", msg);
        }

        HttpResponse
        HttpResponse::internal_error(std::string_view msg)
        {
            return error_page(HttpStatus::internal_error(), "internal error", msg);
        }

        HttpResponse
        HttpResponse::error_page(HttpStatus status, std::string_view title, std::string_view msg)
        {
            std::string code = std::to_string(status.code());

            return HttpResponse(status, ContentType::html,
                                "<!doctype html><html><head><title>" + code + " "
                                + std::string(title) + "</title></head><body><p>"
                                + code + " " + std::string(title) + ": "
                                + html_escape(msg) + "</p></body></html>");
        }
    } /*namespace web*/
} /*namespace xo*/

/* end HttpResponse.cpp */
