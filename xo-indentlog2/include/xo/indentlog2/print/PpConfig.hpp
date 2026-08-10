/** @file PpConfig.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <xo/arena/ArenaConfig.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <cstdint>

namespace xo {
    namespace pp {
        /** @brief layout configuration for PrettySink **/
        class PpLayoutConfig {
        public:
            PpLayoutConfig() = default;
            PpLayoutConfig(uint32_t w, uint32_t srm, uint32_t hrm, uint32_t hn);

            uint32_t indent_width() const { return indent_width_; }
            uint32_t soft_right_margin() const { return soft_right_margin_; }
            uint32_t hard_right_margin() const { return hard_right_margin_; }
            uint32_t hard_max_nesting() const { return hard_max_nesting_; }

            PpLayoutConfig with_indent_width(uint32_t x);
            PpLayoutConfig with_soft_right_margin(uint32_t x);
            PpLayoutConfig with_hard_right_margin(uint32_t x);
            PpLayoutConfig with_hard_max_nesting(uint32_t x);

            friend class PpConfig;

        private:
           /** indent per nesting level **/
            uint32_t indent_width_ = 2;

            /** Target max line length for pretty printing.
             *  Pretty-printer will endeavour to introduce newlines to stay to the left
             *  of this margin
             **/
            uint32_t soft_right_margin_ = 135;

            /** Hard max right margin.
             *  Not used during printing;
             *  determines reserved memory range for token buffer.
             *  Can modify right margin in-place up to (but not beyond) this limit.
             **/
            uint32_t hard_right_margin_ = 2048;

            /** Hard max nesting depth.
             *  Pretty-printing will lose ability to track structure if nesting
             *  level reaches this limit. Memory cost is
             *  sizeof(uint32_t) * hard_max_nesting_.
             **/
            uint32_t hard_max_nesting_ = 1024 * 1024;

        };

        /** @brief log-buffer configuration **/
        class PpLogbufConfig {
        public:
            using ArenaConfig = xo::mm::ArenaConfig;

        public:
            PpLogbufConfig() = default;
            PpLogbufConfig(const ArenaConfig & acfg, bool f);

            const ArenaConfig & logbuf_config() const { return logbuf_config_; }
            bool logbuf_debug_flag() const { return logbuf_debug_flag_; }

            PpLogbufConfig with_logbuf_config(const ArenaConfig & x);
            PpLogbufConfig with_logbuf_debug_flag(bool x);

            PpLogbufConfig with_name(const std::string & name);

            friend class PpConfig;

        private:
            /** configuration for output buffer **/
            ArenaConfig logbuf_config_;

            /** debug flag for @ref PpState::logbuf_ **/
            bool logbuf_debug_flag_ = false;
        };

        /** @brief formatting + resource configuration for PrettySink.
         **/
        class PpConfig {
        public:
            using ArenaConfig = xo::mm::ArenaConfig;
            using size_t = std::size_t;
            using uint32_t = std::uint32_t;

        public:
            PpConfig() = default;
            PpConfig(const PpLayoutConfig & l, const PpLogbufConfig & b, const PpStyle & s);

            /** plain + anonymous **/
            static PpConfig plain();
            /** colored + anonymous **/
            static PpConfig colored();
            /** config for a temporary pretty sink. useful in unit tests **/
            static PpConfig scratch(uint32_t margin);
            /** config for a temporary pretty sink. useful in unit tests **/
            static PpConfig scratch_aux(const std::string & basename, uint32_t margin, const PpStyle & style);

            const PpLayoutConfig & layout() const { return layout_; }
            const PpLogbufConfig & logbuf() const { return logbuf_; }
            const PpStyle & style() const { return style_; }

            PpConfig with_layout(const PpLayoutConfig & x);
            PpConfig with_logbuf(const PpLogbufConfig & x);
            PpConfig with_style(const PpStyle & x);

            PpConfig with_indent_width(uint32_t x);
            PpConfig with_soft_right_margin(uint32_t x);
            PpConfig with_hard_right_margin(uint32_t x);
            PpConfig with_hard_max_nesting(uint32_t x);

            PpConfig with_logbuf_config(const ArenaConfig & x);
            PpConfig with_logbuf_debug_flag(bool x);

            /** hard max arena size **/
            PpConfig with_logbuf_size(size_t);
            PpConfig with_logbuf_name(const std::string & x);

        private:
            /** configuration for layout (no color) **/
            PpLayoutConfig layout_;
            /** configuration for logbuffer **/
            PpLogbufConfig logbuf_;
            /** configuration for universal coloring (shared with FlatSink) **/
            PpStyle style_;
        };

    } /*namespace pp*/
} /*namespace xo*/

/* end PpConfig.hpp */
