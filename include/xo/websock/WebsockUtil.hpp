/* @file WebsockUtil.hpp */

#pragma once

#include <libwebsockets.h>

namespace xo {
    namespace web {
        /* class-as-namespace idiom */
        class WebsockUtil {
        public:
            /* string representation for callback category enum */
            static char const * ws_callback_reason_descr(lws_callback_reasons x);
        }; /*WebsockUtil*/
    } /*namespace web*/
} /*namespace xo*/

/* end WebsockUtil.hpp */
