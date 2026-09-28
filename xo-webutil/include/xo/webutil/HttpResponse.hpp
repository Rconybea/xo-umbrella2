/** @file HttpResponse.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "HttpStatus.hpp"
#include <string>
#include <string_view>

namespace xo {
    namespace web {
        /** content type of an http response **/
        enum class ContentType {
            json,
            html,
            text,
        };

        /** mime type for @p x, e.g. "application/json",
         *  "text/html; charset=utf-8"
         **/
        char const * content_type_str(ContentType x);

        /** @class HttpResponse
         *  @brief what an http endpoint's handler answers: status, content
         *  type, body.
         *
         *  The body is a whole string: the server needs its length for the
         *  headers before it writes them.
         **/
        class HttpResponse {
        public:
            HttpResponse(HttpStatus status, ContentType content_type, std::string body);

            /** ok, application/json **/
            static HttpResponse json(std::string body);
            /** ok, text/html **/
            static HttpResponse html(std::string body);
            /** ok, text/plain **/
            static HttpResponse text(std::string body);

            /** not_found, text/html: @p msg, escaped, in a minimal page **/
            static HttpResponse not_found(std::string_view msg);
            /** forbidden, text/html: likewise **/
            static HttpResponse forbidden(std::string_view msg);
            /** internal_error, text/html: likewise **/
            static HttpResponse internal_error(std::string_view msg);

            HttpStatus status() const { return status_; }
            ContentType content_type() const { return content_type_; }
            std::string const & body() const { return body_; }

        private:
            /** a minimal html page reporting @p status with @p msg **/
            static HttpResponse error_page(HttpStatus status, std::string_view title,
                                           std::string_view msg);

        private:
            HttpStatus status_;
            /** sent as the Content-Type header **/
            ContentType content_type_;
            /** response payload **/
            std::string body_;
        };

        /** @p s with html's special characters (&, <, >, ") escaped **/
        std::string html_escape(std::string_view s);
    } /*namespace web*/
} /*namespace xo*/

/* end HttpResponse.hpp */
