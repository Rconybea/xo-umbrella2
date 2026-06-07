/* file EndpointUtil.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include <string>

namespace xo {
    namespace web {
        class EndpointUtil {
        public:
            /* find fixed prefix for a URI pattern.
             * patterns are used with both http endpoints (see DynamicEndpoint),
             * and stream endpoints (see StreamEndpoint)
             *
             * e.g. stem("/dyn/uls/${ulticker}/snap") => "/dyn/uls/"
             */
            static std::string stem(std::string const & pattern);
        }; /*EndpointUtil*/

    } /*namespace web*/
} /*namespace xo*/

/* end EndpointUtil.hpp */
