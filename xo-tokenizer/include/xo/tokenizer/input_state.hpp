/* @file input_state.hpp
 *
 * author: Roland Conybeare, Jun 2025
 */

#pragma once

#include "span.hpp"

namespace xo {
    namespace scm {
        /** @class input_state
         *  @brief Track detailed input position for use in error messages
         *
         **/
        template <typename CharT>
        class input_state {
        public:
            using span_type = span<const CharT>;

        public:
            input_state() = default;
            explicit input_state(const span<const CharT>& x, size_t cpos, size_t ws)
                : current_line_{x}, current_pos_{cpos}, whitespace_{ws} {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning"
            const span_type & current_line() const { return current_line_; }
#pragma GCC diagnostic pop
            size_t current_pos() const { return current_pos_; }
            size_t whitespace() const { return whitespace_; }

            void capture_current_line(const span_type & input);
            void discard_current_line();

            void consume(size_t z) { current_pos_ += z; }

            void reset_whitespace() { whitespace_ = 0; }
            void increment_whitespace() { ++whitespace_; }

        private:
            /** remember current input line.  Used only to report errors **/
            span<const CharT> current_line_ = span<const CharT>();
            /** current input position within @ref current_line_ **/
            size_t current_pos_ = 0;
            /** whitespace since end of preceding token,
             *  or last newline, whichever is less
             **/
            size_t whitespace_ = 0;

            bool debug_flag_ = false;
        };

        template <typename CharT>
        void
        input_state<CharT>::discard_current_line() {
            this->current_line_ = span_type::make_null();
            this->current_pos_ = 0;
        }

        template <typename CharT>
        void
        input_state<CharT>::capture_current_line(const span_type & input)
        {
            // see also discard_current_line()

            scope log(XO_DEBUG(debug_flag_));

            /* look ahead to {end of line, end of input}, whichever comes first */
            const CharT * sol = input.lo();
            const CharT * eol = sol;

            while ((eol < input.hi()) && (*eol != '\n'))
                ++eol;

            this->current_line_ = span_type(sol, eol);
//            this->current_pos_ = 0;

            log && log(xtag("current_line", print::printspan(current_line_)));
        }
    }
}
