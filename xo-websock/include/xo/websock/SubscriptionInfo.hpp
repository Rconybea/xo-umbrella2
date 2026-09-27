/** @file SubscriptionInfo.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <cstdint>
#include <string>

namespace xo {
    namespace web {
        /** @brief an active subscription, as a plain value: what
         *  WsSessionRouter::subscriptions() reports.  A copy.
         *
         *  Temporary: retired when subscriptions print natively
         *  (.xo-backlog/xo-websock/issues/10, 5d).
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
    } /*namespace web*/
} /*namespace xo*/

/* end SubscriptionInfo.hpp */
