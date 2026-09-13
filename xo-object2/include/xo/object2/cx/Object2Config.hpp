/** @file Object2Config.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/object2/init_object2.hpp"
#include <xo/subsys/AppContext.hpp>

namespace xo {
    /** @brief configuration for subsystem xo-printjson/ **/
    class Object2Config {
    public:
        Object2Config() = default;
    };

    /** xo-printjson contributes a trivial configuration and a context **/
    template <>
    class SubsystemConfig<S_object2_tag> {
    public:
        using Type = Object2Config;
    };
} /*namespace xo*/

/* end Object2Config.hpp */
