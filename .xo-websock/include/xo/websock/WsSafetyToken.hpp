/* file WsSafetyToken.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "SafetyToken.hpp"
#include <mutex>

namespace xo {
    namespace web {
        class WebserverImplWsThread;
        class WebsocketSessionRecd;

        /* only websocket thread can obtain this token */
        class WsSafetyToken : public SafetyToken<class WsSafetyToken_tag> {
        private:
            friend class WebserverImplWsThread;

        private:
            /* only WebserverImpl should construct this */
            WsSafetyToken() = default;
        }; /*WsSafetyToken*/

    } /*namespace web*/
} /*namespace xo*/

/* end WsSafetyToken.hpp */
