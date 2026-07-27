/** @file PpConfig.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpConfig.hpp"

namespace xo {
    using std::uint32_t;

    namespace pp {
        PpConfig::PpConfig(uint32_t w, uint32_t srm, uint32_t hrm, uint32_t hn, const ArenaConfig & logbuf_cfg,
                           bool logbuf_debug_flag)
            : indent_width_{w}, soft_right_margin_{srm}, hard_right_margin_{hrm},
              hard_max_nesting_{hn}, logbuf_config_{logbuf_cfg}, logbuf_debug_flag_{logbuf_debug_flag}
        {}

        PpConfig
        PpConfig::with_indent_width(uint32_t x)
        {
            PpConfig retval = *this;

            retval.indent_width_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_soft_right_margin(uint32_t x)
        {
            PpConfig retval = *this;

            retval.soft_right_margin_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_hard_right_margin(uint32_t x)
        {
            PpConfig retval = *this;

            retval.hard_right_margin_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_hard_max_nesting(uint32_t x)
        {
            PpConfig retval = *this;

            retval.hard_max_nesting_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_logbuf_config(const ArenaConfig & x)
        {
            PpConfig retval = *this;

            retval.logbuf_config_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_logbuf_debug_flag(bool x)
        {
            PpConfig retval = *this;

            retval.logbuf_debug_flag_ = x;

            return retval;
        }

    } /*namespace pp*/
} /*namesapce xo*/

/* end PpConfig.cpp */
