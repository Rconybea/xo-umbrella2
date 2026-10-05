/** @file PrintJsonConfig.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/printjson/init_printjson.hpp"
#include "xo/printjson/PrintJson.hpp"
#include <xo/subsys/AppContext.hpp>
#include <cstdint>

namespace xo {
    /** @brief configuration for subsystem xo-printjson/ **/
    class PrintJsonConfig {
    public:
        PrintJsonConfig() = default;

        /** copy of this config with nesting limit @p z **/
        PrintJsonConfig with_max_depth(std::uint32_t z) const {
            PrintJsonConfig retval = *this;
            retval.max_depth_ = z;
            return retval;
        }

        /** a print aborts rather than nest deeper than this:
         *  see json::PrintJson::max_depth()
         **/
        std::uint32_t max_depth_ = json::PrintJson::c_default_max_depth;
    };

    /** xo-printjson contributes a configuration and a context **/
    template <>
    class SubsystemConfig<S_printjson_tag> {
    public:
        using Type = PrintJsonConfig;
    };
} /*namespace xo*/

/* end PrintJsonConfig.hpp */
