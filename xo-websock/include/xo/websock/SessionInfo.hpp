/** @file SessionInfo.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace xo {
    namespace web {
        /** @brief an active subscription, as a plain value: what
         *  WsSessionRouter::subscriptions() reports.  A copy.
         **/
        struct SubscriptionInfo {
            /* server-assigned, never reused within the session */
            std::uint32_t sub_id_ = 0;
            /* as the client asked for it, e.g. "/demo/1" */
            std::string stream_name_;
            /* the serving endpoint's pattern, e.g. "/demo/${id}" -- identifies
             * the endpoint among those registered
             */
            std::string endpoint_pattern_;
        };

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
            /* active subscriptions, by sub_id; retired sub_ids not listed */
            std::vector<SubscriptionInfo> subscriptions_;
        };
    } /*namespace web*/
} /*namespace xo*/

/* end SessionInfo.hpp */
