/** @file PpConfig.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include <cstdint>

namespace xo {
    namespace print {
        class PpConfig {
        public:
            using uint32_t = std::uint32_t;

        public:
            PpConfig() = default;
            PpConfig(uint32_t w, uint32_t srm, uint32_t hrm, uint32_t hn);

            uint32_t indent_width() const { return indent_width_; }
            uint32_t soft_right_margin() const { return soft_right_margin_; }
            uint32_t hard_right_margin() const { return hard_right_margin_; }
            uint32_t hard_max_nesting() const { return hard_max_nesting_; }

            PpConfig with_soft_right_margin(uint32_t x);
            PpConfig with_hard_right_margin(uint32_t x);
            PpConfig with_hard_max_nesting(uint32_t x);
            PpConfig with_indent_width(uint32_t x);

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
    } /*namespace print*/
} /*namespace xo*/

/* end PpConfig.hpp */
