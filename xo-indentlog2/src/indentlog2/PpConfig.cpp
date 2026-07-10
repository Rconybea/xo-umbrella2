/** @file PpConfig.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpConfig.hpp"

namespace xo {
    using std::uint32_t;

    namespace print {
        PpConfig::PpConfig(uint32_t w, uint32_t srm, uint32_t hrm, uint32_t hn, const ArenaConfig & logbuf_cfg)
            : indent_width_{w}, soft_right_margin_{srm}, hard_right_margin_{hrm},
              hard_max_nesting_{hn}, logbuf_config_{logbuf_cfg}
        {}

        PpConfig
        PpConfig::with_indent_width(uint32_t x)
        {
            return PpConfig{
                x,
                soft_right_margin_,
                hard_right_margin_,
                hard_max_nesting_,
                logbuf_config_,
            };
        }

        PpConfig
        PpConfig::with_soft_right_margin(uint32_t x)
        {
            return PpConfig{
                indent_width_,
                x,
                hard_right_margin_,
                hard_max_nesting_,
                logbuf_config_,
            };
        }

        PpConfig
        PpConfig::with_hard_right_margin(uint32_t x)
        {
            return PpConfig{
                indent_width_,
                soft_right_margin_,
                x,
                hard_max_nesting_,
                logbuf_config_,
            };
        }

        PpConfig
        PpConfig::with_hard_max_nesting(uint32_t x)
        {
            return PpConfig{
                indent_width_,
                soft_right_margin_,
                hard_right_margin_,
                x,
                logbuf_config_,
            };
        }
    } /*namespace print*/
} /*namesapce xo*/

/* end PpConfig.cpp */
