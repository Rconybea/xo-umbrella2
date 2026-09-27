/** @file SessionInfo.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <cstdint>

namespace xo {
    namespace web {
        /** @brief a live websocket session, as a plain value: what
         *  Webserver::sessions() reports.
         *
         *  A copy, taken under the session table's lock; holds no reference
         *  to the session.
         **/
        struct SessionInfo {
            /* from WsSessionTable::next_id(); never reused */
            std::uint64_t session_id_ = 0;
            /* false once the session's sender is closed -- for a session
             * still listed, only in the moment between close and removal
             */
            bool sender_open_ = false;
            /* active subscriptions: retired sub_ids not counted */
            std::uint32_t n_subscription_ = 0;
        };
    } /*namespace web*/
} /*namespace xo*/

/* end SessionInfo.hpp */
