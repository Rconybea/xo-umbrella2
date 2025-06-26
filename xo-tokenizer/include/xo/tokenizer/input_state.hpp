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
            /** @defgroup input-state-type-traits input-state type straits **/
            ///@{

            /** type representing a contiguous span of tokenizer input characters **/
            using span_type = span<const CharT>;

            ///@}

        public:
            /** @defgroup input-state-ctors input_state constructors **/
            ///@{

            input_state() = default;
            explicit input_state(bool debug_flag) : debug_flag_{debug_flag} {}
            /** Create instance with supplied @p current_line, @p current_pos, @p whitespace.
             *  Introduced for unit tests, not used in tokenizer.
             **/
            explicit input_state(const span<const CharT>& current_line, size_t current_pos, size_t whitespace)
                : current_line_{current_line}, current_pos_{current_pos}, whitespace_{whitespace} {}

            ///@}

            /** @defgroup input-state-static-methods input_state static methods **/
            ///@{

            /** recognize the newline character '\n' **/
            static bool is_newline(CharT ch);
            /** identifies whitespace chars.
             *  These are chars that do not belong to any token.
             *  They are not permitted to appear within
             *  a symbol or string token.
             *  Appearance of a whitespace char forces completioon of
             *  preceding token.
             **/
            static bool is_whitespace(CharT ch);

            ///@}

            /** @defgroup input-state-access-methods **/
            ///@{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning"
            const span_type & current_line() const { return current_line_; }
#pragma GCC diagnostic pop
            size_t current_pos() const { return current_pos_; }
            size_t whitespace() const { return whitespace_; }
            bool debug_flag() const { return debug_flag_; }

            ///@}

            /** @defgroup input-state-general-methods **/
            ///@{

            /** capture prefix of @p input up to first newline **/
            void capture_current_line(const span_type & input);

            /** Reset input state for start of next line.
             *  Expression parser may use this to discard remainder of input line
             *  after a parsing error.
             **/
            void discard_current_line();

            /** Add @p z to current position **/
            void consume(size_t z);

            /** Skip prefix of input comprising whitespace.
             *  Return pointer to first non-whitespace character in @p input,
             *  or @c input.hi if input contains only whitespace
             **/
            const CharT * skip_leading_whitespace(const span_type & input);

            ///@}

        private:
            /** @defgroup input-state-instance-vars input_state instance variables **/
            ///@{

            /** remember current input line.  Used only to report errors **/
            span<const CharT> current_line_ = span<const CharT>();
            /** current input position within @ref current_line_ **/
            size_t current_pos_ = 0;
            /** whitespace since end of preceding token,
             *  or last newline, whichever is less
             **/
            size_t whitespace_ = 0;

            /** true to log input activity */
            bool debug_flag_ = false;

            ///@}
        };

        template <typename CharT>
        bool
        input_state<CharT>::is_newline(CharT ch) {
            return (ch == '\n');
        }

        template <typename CharT>
        bool
        input_state<CharT>::is_whitespace(CharT ch) {
            switch(ch) {
            case ' ': return true;
            case '\t': return true;
            case '\n': return true;
            case '\r': return true;
            }

            return false;
        }

        template <typename CharT>
        void
        input_state<CharT>::consume(size_t z) {
            scope log(XO_DEBUG(debug_flag_));

            this->current_pos_ += z;

            log && log(xtag("z", z), xtag("current_pos", current_pos_));
        }

        template <typename CharT>
        void
        input_state<CharT>::discard_current_line() {
            this->current_line_ = span_type::make_null();
            this->current_pos_ = 0;
            this->whitespace_ = 0;
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

            log && log(xtag("current_line", print::printspan(current_line_)));
        }

        template <typename CharT>
        const CharT *
        input_state<CharT>::skip_leading_whitespace(const span_type & input)
        {
            const CharT * ix = input.lo();

            if (this->current_line().is_null()) {
                this->capture_current_line(input);
            }

            this->whitespace_ = 0;

            /* skip whitespace + remember beginning of most recent line */
            while (is_whitespace(*ix) && (ix != input.hi())) {
                if (is_newline(*ix)) {
                    ++ix;

                    this->capture_current_line(span_type(ix, input.hi()));
                } else {
                    ++ix;

                    ++(this->whitespace_);
                }
            }

            return ix;
        }
    }
}
