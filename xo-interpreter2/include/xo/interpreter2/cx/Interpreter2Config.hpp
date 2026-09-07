/** @file Interpreter2Config.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "init_interpreter2.hpp"
#include <xo/subsys/AppContext.hpp>
#include <cstdint>

namespace xo {
    /** @brief configuration for subsystem xo-interpreter2/ **/
    class Interpreter2Config {
    public:
        explicit Interpreter2Config() {}

    private:
    };

    /** xo-interpreter2 contributes a configuration and a context **/
    template <>
    class SubsystemConfig<S_interpreter2_tag> {
    public:
        using Type = Interpreter2Config;
    };
} /*namespace xo*/

/* end Interpreter2Config.hpp */
