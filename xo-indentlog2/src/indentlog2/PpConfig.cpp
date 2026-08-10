/** @file PpConfig.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpConfig.hpp"

namespace xo {
    using std::uint32_t;

    namespace pp {
        // ----- PpLayoutConfig -----

        PpLayoutConfig::PpLayoutConfig(uint32_t w, uint32_t srm, uint32_t hrm, uint32_t hn)
            : indent_width_{w}, soft_right_margin_{srm}, hard_right_margin_{hrm},
              hard_max_nesting_{hn}
        {}

        PpLayoutConfig
        PpLayoutConfig::with_indent_width(uint32_t x)
        {
            PpLayoutConfig retval = *this;

            retval.indent_width_ = x;

            return retval;
        }

        PpLayoutConfig
        PpLayoutConfig::with_soft_right_margin(uint32_t x)
        {
            PpLayoutConfig retval = *this;

            retval.soft_right_margin_ = x;

            return retval;
        }

        PpLayoutConfig
        PpLayoutConfig::with_hard_right_margin(uint32_t x)
        {
            PpLayoutConfig retval = *this;

            retval.hard_right_margin_ = x;

            return retval;
        }

        PpLayoutConfig
        PpLayoutConfig::with_hard_max_nesting(uint32_t x)
        {
            PpLayoutConfig retval = *this;

            retval.hard_max_nesting_ = x;

            return retval;
        }

        // ----- PpLogbufConfig -----

        PpLogbufConfig::PpLogbufConfig(const ArenaConfig & logbuf_cfg, bool logbuf_debug_flag)
            : logbuf_config_{logbuf_cfg}, logbuf_debug_flag_{logbuf_debug_flag}
        {}

        PpLogbufConfig
        PpLogbufConfig::with_logbuf_config(const ArenaConfig & x)
        {
            PpLogbufConfig retval = *this;

            retval.logbuf_config_ = x;

            return retval;
        }

        PpLogbufConfig
        PpLogbufConfig::with_logbuf_debug_flag(bool x)
        {
            PpLogbufConfig retval = *this;

            retval.logbuf_debug_flag_ = x;

            return retval;
        }

        PpLogbufConfig
        PpLogbufConfig::with_name(const std::string & x)
        {
            PpLogbufConfig retval = *this;

            retval.logbuf_config_.name_ = x;

            return retval;
        }

        // ------ PpConfig -----

        PpConfig::PpConfig(const PpLayoutConfig & l, const PpLogbufConfig & b, const PpStyle & s)
          : layout_{l}, logbuf_{b}, style_{s}
        {}

        PpConfig
        PpConfig::scratch_aux(const std::string & basename,
                              uint32_t margin,
                              const PpStyle & style)
        {
            int s_ppconfig_seq{0};

            // its desirable for arena names to be unique,
            // so that they can be distinguished in MemorySizeVisitor pools

            ArenaConfig logbuf_cfg { .name_ = basename + std::to_string(++s_ppconfig_seq),
                                     .size_ = 64*1024 };

            return PpConfig()
                   .with_logbuf_config(logbuf_cfg)
                   .with_soft_right_margin(margin)
                   .with_style(style);
        }

        PpConfig
        PpConfig::scratch_plain(uint32_t margin)
        {
            return scratch_aux("anon", margin, PpStyle::plain());
        }

        PpConfig
        PpConfig::scratch_colored(uint32_t margin)
        {
            return scratch_aux("anon", margin, PpStyle::colored());
        }

        PpConfig
        PpConfig::plain()
        {
            return scratch_plain(135 /*soft_right_margin*/);
        }

        PpConfig
        PpConfig::colored()
        {
            return scratch_colored(135 /*soft_right_margin*/);
        }

        PpConfig
        PpConfig::with_layout(const PpLayoutConfig & x)
        {
            PpConfig retval = *this;

            retval.layout_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_logbuf(const PpLogbufConfig & x)
        {
            PpConfig retval = *this;

            retval.logbuf_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_style(const PpStyle & x)
        {
            PpConfig retval = *this;

            retval.style_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_indent_width(uint32_t x)
        {
            PpConfig retval = *this;

            retval.layout_.indent_width_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_soft_right_margin(uint32_t x)
        {
            PpConfig retval = *this;

            retval.layout_.soft_right_margin_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_hard_right_margin(uint32_t x)
        {
            PpConfig retval = *this;

            retval.layout_.hard_right_margin_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_hard_max_nesting(uint32_t x)
        {
            PpConfig retval = *this;

            retval.layout_.hard_max_nesting_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_logbuf_config(const ArenaConfig & x)
        {
            PpConfig retval = *this;

            retval.logbuf_.logbuf_config_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_logbuf_debug_flag(bool x)
        {
            PpConfig retval = *this;

            retval.logbuf_.logbuf_debug_flag_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_logbuf_size(size_t x)
        {
            PpConfig retval = *this;

            retval.logbuf_.logbuf_config_.size_ = x;

            return retval;
        }

        PpConfig
        PpConfig::with_logbuf_name(const std::string & x)
        {
            PpConfig retval = *this;

            retval.logbuf_.logbuf_config_.name_ = x;

            return retval;
        }

    } /*namespace pp*/
} /*namesapce xo*/

/* end PpConfig.cpp */
