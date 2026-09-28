/** @file HttpRequest.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "Alist.hpp"
#include <string>
#include <string_view>

namespace xo {
    namespace web {
        /** @class HttpRequest
         *  @brief what an http endpoint's handler is asked: the uri, and the
         *  value of each variable in the endpoint's uri pattern.
         *
         *  A class rather than handler arguments, so that more of the request
         *  (query, method, headers) can be offered later without changing any
         *  handler's signature.
         **/
        class HttpRequest {
        public:
            HttpRequest(std::string uri, Alist vars);

            /** the uri, after the server's dynamic mount (e.g. /types for
             *  http://host:port/dyn/types)
             **/
            std::string_view uri() const { return uri_; }

            /** value of pattern variable @p name (e.g. "path" for
             *  ${path...}); empty if the pattern has no such variable
             **/
            std::string_view var(std::string_view name) const;

            /** every pattern variable, in pattern order **/
            Alist const & vars() const { return vars_; }

        private:
            std::string uri_;
            Alist vars_;
        };
    } /*namespace web*/
} /*namespace xo*/

/* end HttpRequest.hpp */
