/* @file input_state.hpp
 *
 * author: Roland Conybeare, Jun 2025
 */

#pragma once

#include "span.hpp"

namespace xo {
    namespace scm {
        /** enum to report outcome of @ref capture_current_line **/
        enum class input_error {
            /** normal return, input line successfully identified and captured **/
            ok = 0,
            /** incomplete input; should not have been submitted to @ref capture_current_line.
             *  note: submit last line of input with eof_flag=true
             **/
            incomplete,
            N
        };

        /** @class input_state
         *  @brief Track detailed input position for use in error messages
         *
         *  input characters fall into two categories:
         *  - consumed: memory can be reclaimed/recycled
         *  - buffered: memory will be retained unaltered until consumed
         *
         *  remarks:
         *  - always in one of two states:
         *    - empty
         *    - contains exactly one line of input
         *  - also record current input position.
         *    Use this for example to identify where tokenizer rejected input.
         *  - .current_pos advances by one token
         *
         *  - buffered characters always form a single contiguous range.
         *  - input_state does not own any storage; storage is owned elsewhere
         *
         *  @text
         *
         *    <------------------.current_line------------------>
         *                                   >  <-- .whitespace
         *    cccccccccccccccccccccccccccccccc__TTTTTTTTxxxxxxxxx
         *    ^                                 ^                ^
         *    .current_line.lo                  |                .current_line.hi
         *                           .current_pos
         *
         *    <----prev_line----> <----current_line---->
         *                                   >  <--whitespace
         *    ppppppppppppppppppp cccccccccccc__TTTTTTTT
         *    ^
         *
         *  @endtext
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
            explicit input_state(const span<const CharT>& current_line,
                                 size_t current_pos,
                                 size_t whitespace) : current_line_{current_line},
                                                      current_pos_{current_pos},
                                                      whitespace_{whitespace} {}

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
#ifndef __APPLE__
#pragma GCC diagnostic ignored "-Wchanges-meaning"
#endif
            const span_type & current_line() const { return current_line_; }
#pragma GCC diagnostic pop
            size_t tk_start() const { return tk_start_; }
            size_t current_pos() const { return current_pos_; }
            size_t whitespace() const { return whitespace_; }
            bool debug_flag() const { return debug_flag_; }

            ///@}

            /** @defgroup input-state-general-methods **/
            ///@{

            /** Input state less @p n chars.
             *  Use to recover input state before a complete but error-triggering token
             **/
            input_state rewind(std::size_t n) const;

            /** Capture prefix of @p input up to first newline.
             *  Set read position to start of line.
             *
             *  Alters:
             *    .current_line
             *    .current_pos
             *
             * Return pair comprising error code and input span representing first line
             * (including trailing newline) from @p input.
             **/
            std::pair<input_error, span_type> capture_current_line(const span_type & input,
                                                                   bool eof_flag);

            /** atomically return current line while discarding it from input state
             *
             *  Alters
             *    .current_line
             *    .current_pos
             *    .whitespace
             **/
            span_type consume_current_line();

            /** Reset input state for start of next line.
             *  Expression parser may use this to discard remainder of input line
             *  after a parsing error.
             *
             * Alters:
             *   .current_line
             *   .current_pos
             *   .whitespace
             **/
            void discard_current_line();

            /** Advance input position by @p z
             *
             *  Alters:
             *   .current_pos
             **/
            void advance(size_t z);

            /** Advance .current_pos to pos.
             *  Require: pos in @ref current_line_
             **/
            void advance_until(const CharT * pos);

            /** Skip prefix of input, starting at current read position,
             *  comprising only whitespace.
             *
             *  Presume input position is at end of token;
             *  on return @ref whitespace_ counts number of whitespace characters
             *  skipped.
             *
             *  Return pointer to first non-whitespace character after @ref current_pos_
             *  or @ref current_line_.hi if reached end of buffered line.
             *
             *  Alters:
             *    .whitespace
             **/
            const CharT * skip_leading_whitespace();

            ///@}

        private:
            /** @defgroup input-state-instance-vars input_state instance variables **/
            ///@{

            /** remember current input line.  Used only to report errors **/
            span<const CharT> current_line_ = span<const CharT>();
            /** start of last token within @ref current_line_ **/
            size_t tk_start_ = 0;
            /** input position within @ref current_line_ **/
            size_t current_pos_ = 0;
            /** number of whitespace chars since end of preceding token,
             *  or last newline, whichever is less
             **/
            size_t whitespace_ = 0;

            /** true to log input activity */
            bool debug_flag_ = false;

            ///@}
        }; /*input_state*/

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
        input_state<CharT>
        input_state<CharT>::rewind(std::size_t n) const {
            return input_state<CharT>(this->current_line_,
                                      (n <= current_pos_) ? current_pos_ - n : 0,
                                      0 /*whitespace*/);
        }

        template <typename CharT>
        void
        input_state<CharT>::advance(size_t z) {
            scope log(XO_DEBUG(debug_flag_));

            this->current_pos_ += z;

            log && log(xtag("z", z), xtag("current_pos", current_pos_));
        }

        template <typename CharT>
        void
        input_state<CharT>::advance_until(const CharT * pos) {
            scope log(XO_DEBUG(debug_flag_));

            assert(current_line_.lo() <= pos && pos <= current_line_.hi());

            this->current_pos_ = pos - current_line_.lo();

            log && log(xtag("current_pos", current_pos_));
        }

        template <typename CharT>
        auto
        input_state<CharT>::consume_current_line() -> span_type {
            span_type retval = current_line_;

            this->discard_current_line();

            return retval;
        }

        template <typename CharT>
        void
        input_state<CharT>::discard_current_line() {
            this->current_line_ = span_type::make_null();
            this->current_pos_ = 0;
            this->whitespace_ = 0;
        }

        template <typename CharT>
        auto
        input_state<CharT>::capture_current_line(const span_type & input,
                                                 bool eof_flag) -> std::pair<input_error, span_type>
        {
            // see also discard_current_line()
            // note: must capture entirety of first line,
            //       for example including leading whitespace.
            //       See discussion in tokenizer scan() method

            scope log(XO_DEBUG(debug_flag_));

            /* look ahead to {end of line, end of input}, whichever comes first */
            const CharT * sol = input.lo();
            const CharT * eol = sol;

            if (sol == current_line_.lo()) {
                log && log("short-circuit - current line already stashed");

                /* nothing to do here */
                return std::make_pair(input_error::ok, current_line_);
            }

            while ((eol < input.hi()) && (*eol != '\n'))
                ++eol;

            if (*eol == '\n') {
                /* include \n at end-of-line */
                ++eol;
            } else {
                if (!eof_flag) {
                    /* caller expected to provide complete line of input. complain and ignore */
                    return std::make_pair(input_error::incomplete,
                                          input.prefix(0ul));
                }
            }

            this->current_line_ = span_type(sol, eol);
            this->current_pos_ = 0;
            this->whitespace_ = 0;

            log && log(xtag("current_line", print::printspan(current_line_)),
                       xtag("current_pos", current_pos_));

            return std::make_pair(input_error::ok,
                                  span_type(sol, eol));
        }

        template <typename CharT>
        const CharT *
        input_state<CharT>::skip_leading_whitespace()
        {
            scope log(XO_DEBUG(debug_flag_));

            const CharT * ix = current_line_.lo() + current_pos_;

            this->whitespace_ = 0;

            /* skip whitespace + remember beginning of most recent line */
            while (is_whitespace(*ix) && (ix != current_line_.hi())) {
                ++ix;

                ++(this->whitespace_);
            }

            this->tk_start_ = ix - current_line_.lo();
            this->current_pos_ = ix - current_line_.lo();

            return ix;
        }

        template <typename CharT>
        inline std::ostream &
        operator<<(std::ostream & os,
                   const input_state<CharT>& x)
        {
            using xo::print::unq;

            os << "<input_state"
            << xtag("tk", x.tk_start())
            << xtag("pos", x.current_pos())
            << xtag("line", unq(std::string_view(x.current_line().lo(), x.current_line().hi())))
            << xtag("whitespace", x.whitespace())
            << ">";

            return os;
        }
    }
}
