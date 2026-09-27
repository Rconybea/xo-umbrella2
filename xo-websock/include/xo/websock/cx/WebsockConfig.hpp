/** @file WebsockConfig.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/websock/init_websock.hpp"
#include <xo/subsys/AppContext.hpp>

namespace xo {
    /** @brief configuration for subsystem xo-websock/ **/
    class WebsockConfig {
    public:
        WebsockConfig() = default;
    };

    /** xo-websock contributes a trivial configuration and a context **/
    template <>
    class SubsystemConfig<S_websock_tag> {
    public:
        using Type = WebsockConfig;
    };
} /*namespace xo*/

/* end WebsockConfig.hpp */
