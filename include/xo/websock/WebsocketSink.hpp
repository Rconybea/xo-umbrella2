/* file WebsocketSink.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "xo/reactor/AbstractSink.hpp"
#include "xo/printjson/PrintJson.hpp"

namespace xo {
    namespace web {
        class Webserver;

        class WebsocketSink : public reactor::AbstractSink {
        public:
            using PrintJson = xo::json::PrintJson;

        public:
            static rp<WebsocketSink> make(rp<Webserver> const & websrv,
                                          rp<PrintJson> const & pjson,
                                          uint32_t session_id,
                                          std::string const & stream_name);
        }; /*WebsocketSink*/
    } /*namespace web*/
} /*namespace xo*/

/* end WebsocketSink.hpp */
