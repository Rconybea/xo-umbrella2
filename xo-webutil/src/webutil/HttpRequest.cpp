/** @file HttpRequest.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "HttpRequest.hpp"
#include <utility>

namespace xo {
    namespace web {
        HttpRequest::HttpRequest(std::string uri, Alist vars)
            : uri_{std::move(uri)},
              vars_{std::move(vars)}
        {}

        std::string_view
        HttpRequest::var(std::string_view name) const
        {
            return vars_.lookup(std::string(name));
        }
    } /*namespace web*/
} /*namespace xo*/

/* end HttpRequest.cpp */
