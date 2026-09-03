/** @file Interpreter2Appcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "Interpreter2Config.hpp"
#include <xo/subsys/AppContext.hpp>

namespace xo {
    /** @brief application context for the xo-interpreter2 subsystem **/
    class Interpreter2Appcx {
    public:
        template <typename Deps>
        Interpreter2Appcx(Deps & /*deps*/,
                          const Interpreter2Config & cfg) : config_{cfg}
        {}

    private:
        /** xo-interpreter2/ configuration **/
        Interpreter2Config config_;
    };

    template <>
    class SubsystemContext<S_interpreter2_tag> {
    public:
        using Type = Interpreter2Appcx;
    };
} /*namespace xo*/

/* end Interpreter2Appcx.hpp */
