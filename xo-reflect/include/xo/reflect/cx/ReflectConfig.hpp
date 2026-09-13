/** @file ReflectConfig.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/reflect/init_reflect.hpp"
#include <xo/subsys/AppContext.hpp>

namespace xo {
    /** @brief Application-level state for the reflect subsystem
     *
     *  Placeholder: may have capacity later, if/when refactor reflect
     *  to use DArena
     **/
    class ReflectConfig {
    public:
        ReflectConfig() = default;
    };

    /** xo-reflect contributes a trivial configuration and a context **/
    template <>
    class SubsystemConfig<S_reflect_tag> {
    public:
        using Type = ReflectConfig;
    };
} /*namespace xo*/

/* end ReflectConfig.hpp */
