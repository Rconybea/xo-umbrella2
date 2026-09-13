/** @file PrintJsonConfig.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/printjson/init_printjson.hpp"
#include <xo/subsys/AppContext.hpp>

namespace xo {
    /** @brief configuration for subsystem xo-printjson/ **/
    class PrintJsonConfig {
    public:
        PrintJsonConfig() = default;
    };

    /** xo-printjson contributes a trivial configuration and a context **/
    template <>
    class SubsystemConfig<S_printjson_tag> {
    public:
        using Type = PrintJsonConfig;
    };
} /*namespace xo*/

/* end PrintJsonConfig.hpp */
