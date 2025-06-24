/* file tokenizer.hpp
 *
 * author: Roland Conybeare, Jul 2024
 */

#pragma once

#include "token.hpp"
#include "span.hpp"
#include "scan_result.hpp"
#include "xo/indentlog/scope.hpp"
#include <cassert>

namespace xo {
    namespace scm {
        /** @class tokenizer
         *  @brief Parse a Schematika character stream into lexical tokens
         *
         *  Use:
         *
         *  @code
         *    // see xo-tokenizer/example/tokenrepl/tokenrepl.cpp
         *    // for exact working code
         *
         *    using tokenizer_type = tokenizer<char>;
         *    using span_type = tokenizer_type::span_type;
         *
         *    tokenizer_type tkz;
         *    span_type input = ...;
         *
         *    while (!input.empty()) {
         *        auto res = tkz.scan(input);
         *        auto [tk, consumed, error] = res.first;
         *
         *        // do something with tk if tk.is_valid()
         *
         *        input = tkz.consume(res.second, input);
         *    }
         *
         *    if endofinput {
         *        auto [tk, consumed, error] = tzk.notify_eof()
         *
         *        // do something with (final) tk if tk.is_valid()
         *    }
         *
         *  @endcode
         *
         * See tokentype.hpp for token types
         **/
        template <typename CharT>
        class tokenizer {
        public:
            using token_type = token<CharT>;
            using error_type = tokenizer_error<CharT>;
            using span_type = span<const CharT>;
            using result_type = scan_result<CharT>;

        public:
            tokenizer(bool debug_flag = false);

            /** recognize the newline character '\n' **/
            bool is_newline(CharT ch) const;

            /** identifies whitespace chars.
             *  These are chars that do not belong to any token.
             *  They are not permitted to appear within
             *  a symbol or string token.
             *  Appearance of a whitespace char forces completioon of
             *  preceding token.
             **/
            bool is_whitespace(CharT ch) const;

            /** identifies punctuation chars.
             *  These are chars that are not permitted to appear within
             *  a symbol token.  Instead they force completion of
             *  a preceding token,  and start a new token with themselves
             **/
            bool is_1char_punctuation(CharT ch) const;

            /** more-relazed version of is_1char_punctuation.
             *  Chars that are not permitted to appear within a symbol token,
             *  but may form token combined with next character
             **/
            bool is_2char_punctuation(CharT ch) const;

            /** true if tokenizer contains stored prefix of
             *  possibly-incomplete token
             **/
            bool has_prefix() const { return !prefix_.empty(); }

            /** assemble token from text @p token_text.
             *  @p token_text will often but not always represent a subset of @p input.
             *  (For example consider multi-line string literals)
             *  Also the span @p token_text may (in uncommon cases)
             *  have been copied to separate storage from @p input
             *
             *  @p initial_whitespace   Amount of whitespace input being consumed from input.
             *  @p initial_token_prefix_from_input  Amount of non-whitespace input being
             *  consumed from input. Not counting any stashed-and-already-consumed input
             *
             *  retval.consumed will represent some possibly-empty prefix of @p input
             **/
            result_type assemble_token(std::size_t initial_whitespace,
                                       std::size_t initial_token_prefix_from_input,
                                       const span_type & token_text,
                                       const span_type & input) const;

            /** degenerate version of assemble_token() on reaching end-of-file **/
            result_type assemble_final_token(const span_type & token_text) const;

            /** scan for next input token,  given @p input.
             *  Note:
             *  - tokenizer can consume input (e.g. whitespace)
             *    without completing a token
             *  - input will remember the extent of the last line of input
             *    for which parsing has begun, but not completed.
             *    It's required that at least that portion of the input span
             *    remain valid across scan(), scan2() calls
             *
             *  @return {parsed token, consumed span}
             **/
            result_type scan(const span_type & input);

            /** When eof is false, same as scan(input).
             *  When eof is true and scan(input) does not report a token,
             *  return notify_eof()
             **/
            result_type scan2(const span_type & input, bool eof);

            /** @retval span with @p consumed permanently removed from @p input.
             *
             *  Purpose of this method is to update @ref current_pos_.
             **/
            span_type consume(const span_type & consumed, const span_type & input);

            /** discard current line after error.  Just cleans up error-reporting state **/
            void discard_current_line();

            /** notify end of input,  resolving any ambiguous input stashed in .prefix
             **/
            result_type notify_eof(const span_type & input);

        private:
            void capture_current_line(const span_type & input);

            result_type scan_completion(const span_type & whitespace,
                                        const CharT* token_end,
                                        const span_type & input);

        private:
            /** true to log tokenizer activity to stdout **/
            bool debug_flag_ = false;
            /** remember current input line.  Used only to report errors **/
            span_type current_line_ = span_type::make_null();
            /** current input position within @ref current_line_ **/
            size_t current_pos_ = 0;
            /** Accumulate partial token here.
             *  This will happen if input sent to @ref tokenizer::scan
             *  ends without a determinate token boundary.
             **/
            std::string prefix_;
        }; /*tokenizer*/

        template <typename CharT>
        tokenizer<CharT>::tokenizer(bool debug_flag)
            : debug_flag_{debug_flag}
        {}

        template <typename CharT>
        bool
        tokenizer<CharT>::is_newline(CharT ch) const {
            return (ch == '\n');
        }

        template <typename CharT>
        bool
        tokenizer<CharT>::is_whitespace(CharT ch) const {
            switch(ch) {
            case ' ': return true;
            case '\t': return true;
            case '\n': return true;
            case '\r': return true;
            }

            return false;
        }

        template <typename CharT>
        bool
        tokenizer<CharT>::is_1char_punctuation(CharT ch) const {
            switch(ch) {
            case '<':
                return true;
            case '>':
                /* can't be punctuation
                 * - appears in tk_yields token: ->
                 */
                return false;
            case '(':
                return true;
            case ')':
                return true;
            case '[':
                return true;
            case ']':
                return true;
            case '{':
                return true;
            case '}':
                return true;
            case ',':
                return true;
            case ';':
                return true;
            case ':':
                /* can't be 1char punctuation -- can begin assignment token */
                return false;
            case '=':
                return true;
            case '-':
                /* can't be punctuation
                 * - can appear inside f64 token: e.g. 1.23e-9.
                 * - begins tk_yields token: ->
                 */
                return false;
            case '+':
                /* can't be punctuation -- can appear inside f64 token: e.g. 1.23e+4 */
                return false;
            case '*':
                /* not punctuation -- allowed in symbol */
                return false;
            case '/':
                /* not punctuation -- for symmetry with +,- */
                return false;
            case '.':
                /* can't be punctuation -- can appear inside f64 token: e.g. 1.23 */
                return false;
            }

            return false;
        }

        template <typename CharT>
        bool
        tokenizer<CharT>::is_2char_punctuation(CharT ch) const {
            /* can't put '-' here, because of the way it appears in numeric literals
             * characters here may not appear in symbol names
             */

            switch(ch) {
            case ':':
                /* can begin := */
                return true;
            }

            return false;
        }

        template <typename CharT>
        auto
        tokenizer<CharT>::assemble_token(std::size_t initial_whitespace,
                                         std::size_t initial_token_prefix_from_input,
                                         const span_type & token_text,
                                         const span_type & input) const -> result_type
        {
            /* literal|pretty|streamlined */
            log_config::style = function_style::streamlined;

            scope log(XO_DEBUG(debug_flag_));
            log && log(xtag("token_text", token_text),
                       xtag("initial_whitespace", initial_whitespace),
                       xtag("initial_token_prefix_from_input", initial_token_prefix_from_input),
                       xtag("input", input));

            tokentype tk_type = tokentype::tk_invalid;
            std::string tk_text;

            const CharT * tk_start = token_text.lo();
            const CharT * tk_end = token_text.hi();

            const CharT * ix = tk_start;

            /* switch here applies to the first character in a token */
            switch (*ix) {
            case '-':
            case '+':
                if (token_text.size() == 1) {
                    /* standalone '+' or '-' */
                    if (*ix == '+')
                        tk_type = tokentype::tk_plus;
                    else if(*ix == '-')
                        tk_type = tokentype::tk_minus;
                }

                /** fall through to numeric literal code below **/
                [[fallthrough]];
            case '.':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                /* examples of valid floating-point numbers:
                 *   .0
                 *   1e0
                 *   1e
                 *   0.
                 *   +1e0
                 *   -1e0
                 *   +1E+2
                 *   -1E+2
                 *   -0.123e-10
                 * non-examples:
                 *   .
                 *   -
                 *   +
                 *   e0
                 *   .e0
                 *   -.e-0
                 *   +.e+0
                 *
                 * in particular: to be recognized as a number,
                 * must contain at least one digit
                 */

                log && log("possible number-token");

                /* true if initial sign -/+ encountered */
                bool sign_flag = false;
                /* true if '.' encountered */
                bool period_flag = false;
                /* true if 'e' | 'E' encountered.
                 */
                bool exponent_flag = false;
                /* true when sign '-' | '+' precedes exponenct digits */
                bool exponent_sign_flag = false;
                /* true when at least one digit follows exponent marker */
                bool exponent_digit_flag = false;
                /* true if at least one digit encountered */
                bool number_flag = false;

                log && log(xtag("*ix", *ix),
                           xtag("tk.length", token_text.size()));
                if (log && (ix + 1 < tk_end))
                    log(xtag("*(ix+1)", *(ix + 1)));

                if ((*ix == '-') && (ix + 2 == token_text.hi()) && (*(ix + 1) == '>')) {
                    /* composing exactly '->' */
                    tk_type = tokentype::tk_yields;
                } else {
                    /* token (if valid) will be one of: {tk_i64, tk_f64, tk_dot}: */
                    for (; ix != token_text.hi(); ++ix) {
                        if ((*ix == '-') || (*ix == '+')) {
                            /* sign allowed:
                             * 1. before period and before first digit
                             * 2. after exponent
                             */
                            if (!period_flag && !number_flag && !sign_flag) {
                                sign_flag = true;
                            } else if (exponent_flag && !exponent_digit_flag) {
                                exponent_sign_flag = true;
                            } else {
                                return result_type::make_error
                                    (error_type(__FUNCTION__ /*src_function*/,
                                                "improperly placed sign indicator",
                                                current_line_,
                                                current_pos_,
                                                initial_whitespace,
                                                (ix - tk_start)));
                            }
                        } else if (*ix == '.') {
                            if (period_flag) {
                                return result_type::make_error
                                    (error_type(__FUNCTION__ /*src_function*/,
                                                "duplicate decimal point in numeric literal",
                                                current_line_,
                                                current_pos_,
                                                initial_whitespace,
                                                (ix - tk_start)));
                            }

                            period_flag = true;
                        } else if ((*ix == 'e') || (*ix == 'E')) {
                            if (exponent_flag) {
                                return result_type::make_error
                                    (error_type(__FUNCTION__ /*src_function*/,
                                                "duplicate exponent marker in numeric literal",
                                                current_line_,
                                                current_pos_,
                                                initial_whitespace,
                                                (ix - tk_start)));
                            }

                            exponent_flag = true;
                        } else if (isdigit(*ix)) {
                            if (exponent_flag) {
                                /* need digit before exponent to recognize as number */
                                exponent_digit_flag = true;
                            } else {
                                number_flag = true;
                            }
                        } else {
                            return result_type::make_error
                                (error_type(__FUNCTION__ /*src_function*/,
                                            "unexpected character in numeric constant" /*error_description*/,
                                            current_line_,
                                            current_pos_,
                                            initial_whitespace,
                                            (ix - tk_start)));
                        }
                    }

                    if (number_flag) {
                        if (period_flag || exponent_flag) {
                            tk_type = tokentype::tk_f64;
                        } else {
                            tk_type = tokentype::tk_i64;
                        }
                    } else if (period_flag && !exponent_flag) {
                        tk_type = tokentype::tk_dot;
                    } else {
                        /* not a valid token */
                    }

                    log && log(xtag("sign_flag", sign_flag));
                    log && log(xtag("period_flag", period_flag),
                               xtag("exponent_flag", exponent_flag),
                               xtag("exponent_sign_flag", exponent_sign_flag),
                               xtag("number_flag", number_flag));
                    log && log(xtag("tk_type", tk_type));
                }

                break;
            }
            case '*':
                if (token_text.size() == 1) {
                    /* standalone '*' */
                    tk_type = tokentype::tk_star;
                    ++ix;
                } else {
                    /* '*' isn't punctuation -- but may allow appearance in a longer token
                     *
                     * thinking that x*y is a symbol with an embedded '*' character;
                     * in particular want to support kebab-case symbols like 'foo-config'
                     */
                }
                break;
            case '/':
                if (token_text.size() == 1) {
                    /* standalone '/' */
                    tk_type = tokentype::tk_slash;
                    ++ix;
                }
                break;
            case '"':
            {
                log && log("recognize string-token");

                tk_type = tokentype::tk_string;

                tk_text.reserve(token_text.hi() - token_text.lo());

                ++ix; /*skip initial " char*/

                /* true on final " */
                bool endofstring = false;

                for (; ix != token_text.hi(); ++ix) {
                    log && log(xtag("*ix", *ix));

                    switch(*ix) {
                    case '"':
                        endofstring = true;

                        /* skip final " char, don't capture */
                        ++ix;

                        break;
                    case '\\':
                        /* skip escape char, don't capture */
                        ++ix;

                        if (ix == token_text.hi()) {
                            return result_type::make_error
                                (error_type(__FUNCTION__ /*src_function*/,
                                            "expecting key following escape character \\",
                                            current_line_,
                                            current_pos_,
                                            initial_whitespace,
                                            (ix - tk_start)));
                        }

                        switch(*ix) {
                        case '\\':
                            log && log(xtag("*ix", *ix), xtag("escaped", "t"));
                            tk_text.push_back(*ix);
                            break;
                        case 'n':
                            log && log(xtag("*ix", *ix), xtag("newline", "t"));
                            tk_text.push_back('\n');
                            break;
                        case 't':
                            log && log(xtag("*ix", *ix), xtag("tab", "t"));
                            tk_text.push_back('\t');
                            break;
                        case 'r':
                            log && log(xtag("*ix", *ix), xtag("cr", "t"));
                            tk_text.push_back('\r');
                            break;
                        case '"':
                            log && log(xtag("*ix", *ix), xtag("quote", "t"));
                            tk_text.push_back('"');
                            break;
                        default:
                            return result_type::make_error
                                (error_type(__FUNCTION__ /*src_function*/,
                                            "expecting one of n|r|\"|\\ following escape \\",
                                            current_line_,
                                            current_pos_,
                                            initial_whitespace,
                                            (ix - tk_start)));
                        }
                        break;
                    default:
                        tk_text.push_back(*ix);
                        break;
                    }

                    if (endofstring)
                        break;
                }

                if (!endofstring) {
                    return result_type::make_error
                        (error_type(__FUNCTION__ /*src_function*/,
                                    "missing terminating '\"' to complete literal string",
                                    current_line_,
                                    current_pos_,
                                    initial_whitespace,
                                    (ix - tk_start)));
                }

                log && log(tostr("tokenizer::assemble_token",
                                 xtag("tk_text", tk_text)));

                break;
            }
            case 'a': case 'A':
            case 'b': case 'B':
            case 'c': case 'C':
            case 'd': case 'D':
            case 'e': case 'E':
            case 'f': case 'F':
            case 'g': case 'G':
            case 'h': case 'H':
            case 'i': case 'I':
            case 'j': case 'J':
            case 'k': case 'K':
            case 'l': case 'L':
            case 'm': case 'M':
            case 'n': case 'N':
            case 'o': case 'O':
            case 'p': case 'P':
            case 'q': case 'Q':
            case 'r': case 'R':
            case 's': case 'S':
            case 't': case 'T':
            case 'u': case 'U':
            case 'v': case 'V':
            case 'w': case 'W':
            case 'x': case 'X':
            case 'y': case 'Y':
            case 'z': case 'Z':
            {
                /* symbol/identifier must begin with a letter?
                 * we want to accept some other chars too.
                 * specifically want to allow identifiers:
                 *   this-is-the-way
                 *   this+is+also+the+way
                 *   how/much/is/that/doggy
                 *   put*an*asterisk*in*that
                 *   something%special%
                 *
                 * like pure lisp,  we don't allow:
                 * - identifier beginning with digit
                 * - period .
                 *
                 * unlike pure lisp,  we don't allow anywhere in a symbol:
                 * - colon     :
                 * - semicolon ;
                 * - comma     ,
                 *
                 * also we don't allow symbols to begin with special chars
                 */

                tk_type = tokentype::tk_symbol;
                break;
            }
            case '<':
                tk_type = tokentype::tk_leftangle;
                ++ix;
                break;
            case '>':
                tk_type = tokentype::tk_rightangle;
                ++ix;
                break;
            case '(':
                tk_type = tokentype::tk_leftparen;
                ++ix;
                break;
            case ')':
                tk_type = tokentype::tk_rightparen;
                ++ix;
                break;
            case '[':
                tk_type = tokentype::tk_leftbracket;
                ++ix;
                break;
            case ']':
                tk_type = tokentype::tk_rightbracket;
                ++ix;
                break;
            case '{':
                tk_type = tokentype::tk_leftbrace;
                ++ix;
                break;
            case '}':
                tk_type = tokentype::tk_rightbrace;
                ++ix;
                break;
            case ',':
                tk_type = tokentype::tk_comma;
                ++ix;
                break;
            case ';':
                tk_type = tokentype::tk_semicolon;
                ++ix;
                break;
            case ':':
            {
                log && log("colon or assignment token");

                if (*(ix + 1) == '=') {
                    tk_type = tokentype::tk_assign;
                    ++ix;
                    ++ix;
                } else {
                     tk_type = tokentype::tk_colon;
                     ++ix;
                }
                break;
            }
            case '=':
                tk_type = tokentype::tk_singleassign;
                ++ix;
                break;
            default:
                break;
            }

            if (tk_type == tokentype::tk_invalid) {
                return result_type::make_error
                    (error_type(__FUNCTION__ /*src_function*/,
                                "illegal input character",
                                current_line_,
                                current_pos_,
                                initial_whitespace,
                                (ix - tk_start)));
            }

            if ((tk_type == tokentype::tk_i64)
                || (tk_type == tokentype::tk_f64)
                || (tk_type == tokentype::tk_symbol))
            {
                /* note: capturing token text here;
                 *       for numeric literals will re-parse in token::i64_value() / token::f64_value()
                 */
                tk_text = std::string(tk_start, tk_end);
            } else if (tk_type == tokentype::tk_string) {
                ; /* nothing to do here -- desired tk_text already constructed */
            }

            if (tk_type == tokentype::tk_symbol) {
                /* check for keywords */

                bool keep_text = false;

                if (tk_text == "type") {
                    tk_type = tokentype::tk_type;
                } else if (tk_text == "def") {
                    tk_type = tokentype::tk_def;
                } else if (tk_text == "lambda") {
                    tk_type = tokentype::tk_lambda;
                } else if (tk_text == "if") {
                    tk_type = tokentype::tk_if;
                } else if (tk_text == "let") {
                    tk_type = tokentype::tk_let;
                } else if (tk_text == "in") {
                    tk_type = tokentype::tk_in;
                } else if (tk_text == "end") {
                    tk_type = tokentype::tk_end;
                } else {
                    /* keep as symbol */
                    keep_text = true;
                }

                if (!keep_text)
                    tk_text.clear();
            }

            return result_type(token_type(tk_type, std::move(tk_text)),
                               input.prefix(initial_whitespace + initial_token_prefix_from_input));
        } /*assemble_token*/

        template <typename CharT>
        auto
        tokenizer<CharT>::assemble_final_token(const span_type & token_text) const -> result_type
        {
            return assemble_token(0 /*initial_whitespace*/,
                                  0 /*initial_token_prefix_from_input*/,
                                  token_text,
                                  span_type::make_null());
        }

        template <typename CharT>
        auto
        tokenizer<CharT>::scan_completion(const span_type & whitespace,
                                          const CharT* token_end,
                                          const span_type & input) -> result_type {

            auto token_span = input.after_prefix(whitespace).prefix_upto(token_end);

            if (this->prefix_.empty()) {
                return assemble_token(whitespace.size(),
                                      token_span.size() /*initial_token_prefix_from_input*/,
                                      token_span,
                                      input);
            } else {
                /* whatever we stashed in .prefix_, should be consumed from input.
                 * control here implies reached end of input with either
                 * - input for which parsing outcome depends on existence of more input,
                 *   and presence of eof now resolves
                 * - malformed input (that might represent prefix of a valid token.  Say "#incl" in C)
                 *
                 * That means stashed .prefix will represent copied range of characters that
                 * ends at the same position as input
                 */
                return result_type::make_partial(input);
            }

        }

        template <typename CharT>
        void
        tokenizer<CharT>::capture_current_line(const span_type & input)
        {
            // see discard_current_line()

            scope log(XO_DEBUG(debug_flag_));

            /* look ahead to {end of line, end of input}, whichever comes first */
            const CharT * sol = input.lo();
            const CharT * eol = sol;

            while ((eol < input.hi()) && (*eol != '\n'))
                ++eol;

            this->current_line_ = span_type(sol, eol);
            this->current_pos_ = 0;

            log && log(xtag("current_line", print::printspan(current_line_)));
        }

        template <typename CharT>
        auto
        tokenizer<CharT>::scan(const span_type & input) -> result_type
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("input", input));

            const CharT * ix = input.lo();

            if (this->current_line_.is_null()) {
                this->capture_current_line(input);
            }

            /* skip whitespace + remember beginning of most recent line */
            while (is_whitespace(*ix) && (ix != input.hi())) {
                if (is_newline(*ix)) {
                    ++ix;

                    this->capture_current_line(span_type(ix, input.hi()));
                } else {
                    ++ix;

#ifdef OBSOLETE
                    ++(this->current_pos_);
#endif
                }
            }

            if(ix == input.hi()) {
                /* no-op */
                return result_type::make_whitespace(input.prefix_upto(ix));
            }

            // TODO:
            // 1. hoist complete_flag up here
            // 2. use in each branch
            // 3. common check for prefix-capturing after if-cascade below done

            /* here: *ix is not whitespace */

            auto whitespace = input.prefix_upto(ix);

            log && log(xtag("whitespace.size", whitespace.size()));

            /* tk_start points to known beginning of token
             * (after any whitespace)
             *
             * goal is to leave ix pointing to 1 char past the end of the token
             */
            const CharT * tk_start = ix;

            if (is_1char_punctuation(*ix)) {
                /* 1-character token */
                ++ix;
            } else if (is_2char_punctuation(*ix)) {
                CharT ch1 = *ix;

                (void)ch1;

                ++ix;

                if (ix == input.hi()) {
                    /* need more input to know if/when token complete */
                    this->prefix_ += std::string(tk_start, input.hi());

                    log && log(xtag("captured-prefix1", this->prefix_));
                } else {
                    CharT ch2 = *ix;

                    if (((ch2 >= '0') && (ch2 <= '9'))
                        || ((ch2 >= 'A') && (ch2 <= 'Z'))
                        || ((ch2 >= 'a') && (ch2 <= 'z')))
                    {
                        /* treat as 1 char punctuation */
                        ;
                    } else {
                        /* include next char */
                        ++ix;
                    }
                }
            } else if (*ix == '"') {
                bool complete_flag = false;

                /* 1. embedded space/tab allowed in string literal.
                 * 2. embedded newline/cr not allowed.
                 */
                CharT prev_ch = '"';

                ++ix;

                for (; ix != input.hi(); ++ix) {
                    /* looking for unescaped " char to end literal */
                    if (*ix == '"') {
                        if (prev_ch != '\\') {
                            ++ix;  /* include terminating " for assemble_token */
                            complete_flag = true;
                            break;
                        }
                    } else if ((*ix == '\n') || (*ix == '\r')) {
                        return result_type::make_error
                            (error_type(__FUNCTION__ /*src_function*/,
                                        "must use \\n or \\r to encode newline/cr in string literal",
                                        current_line_, current_pos_,
                                        whitespace.size(),
                                        (ix - tk_start)));
                    }

                    prev_ch = *ix;
                }

                if (!complete_flag) {
                    /* need more input to know if/when token complete */
                    this->prefix_ += std::string(tk_start, input.hi());

                    log && log(xtag("captured-prefix2", this->prefix_));
                }
            } else {
                /* ix is start of some token */

                if (*ix == '-') {
                    /* this section load-bearing for input '->' scanning from beginning of token */
                    ++ix;

                    if (ix == input.hi()) {
                        /* need more input to know if/when token complete -- see captured-prefix5 below */
                    } else {
                        CharT ch2 = *ix;

                        if (ch2 == '>') {
                            /* include next char and complete token */
                            ++ix;

                            return scan_completion(whitespace, ix /*token_end*/, input);
                        }

                        /* here: -123, -.5e-21 for example */
                    }
                } else if (*ix == '>') {
                    /* this section load-bearing for input '>=' scanning from beginning of token.
                     * Need this because '>' necessarily excluded from is_1char_punctuation()
                     */
                    ++ix;

                    if (ix == input.hi()) {
                        /* need more input to know if/when token complete -- see captured-prefix5 below */
                    } else {
                        CharT ch2 = *ix;

                        if (ch2 != '=') {
                            /* ignore next char and complete token */
                            return scan_completion(whitespace, ix /*token_end*/, input);
                        }

                        /* here: >= for example */
                    }
                }

                /* scan until:
                 * - whitespace
                 * - punctuation
                 */
                for (; ix != input.hi(); ++ix) {
                    if (is_whitespace(*ix)
                        || is_1char_punctuation(*ix)
                        || is_2char_punctuation(*ix))
                    {
                        break;
                    }

                    /* this section load-bearing for input '>' after beginning of a token, e.g. p> */
                    if ((ix > tk_start) && (*ix == '>'))
                        break;

                    /* this section load-bearing for input '->' at the end of another token, e.g. p->q */
                    if (*ix == '-') {
                        if (ix + 1 == input.hi()) {
                            /* need more input to know if/when token complete
                             *
                             *   apple-banana   parses as: {tk_symbol: apple-banana}
                             *   apple->        parses as: {tk_symbol: apple} {tk_yields}
                             *   apple-         illegal (may not end symbol with '-')
                             */
                            break;
                        }

                        if (*(ix + 1) == '>') {
                            /* treat '->' as punctuation;  complete preceding token */
                            break;
                        }
                    }
                }

                if (ix == input.hi()) {
                    /* need more input to know if/when token complete */
                    this->prefix_ += std::string(tk_start, input.hi());

                    log && log(xtag("captured-prefix5", this->prefix_));
                }
            }

            return scan_completion(whitespace, ix /*token_end*/, input);
        } /*scan*/

        template <typename CharT>
        auto
        tokenizer<CharT>::scan2(const span_type & input, bool eof) -> result_type {
            scope log(XO_DEBUG(debug_flag_));

            auto sr = this->scan(input);

            if (sr.is_token() || sr.is_error() || !eof)
                return sr;

            /* control here only if input contains no unambiguous tokens.
             * This implies it contains _at most one_ final token.
             */

            span_type input2 = input.after_prefix(sr.consumed());

            /* need to include src.consumed() in retval */

            auto sr2 = this->notify_eof(input2);

            return result_type(sr2.get_token(),
                               span_type::concat(sr.consumed(), sr2.consumed()),
                               sr2.error());
        }

        template <typename CharT>
        auto
        tokenizer<CharT>::consume(const span_type & consumed, const span_type & input) -> span_type
        {
            this->current_pos_ += consumed.size();

            return input.after_prefix(consumed);
        }

        template <typename CharT>
        void
        tokenizer<CharT>::discard_current_line()
        {
            // see capture_current_line()

            this->current_line_ = span_type::make_null();
            this->current_pos_ = 0;
        }

        template <typename CharT>
        auto
        tokenizer<CharT>::notify_eof(const span_type & input) -> result_type {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("prefix_", prefix_), xtag("prefix_.size", prefix_.size()), xtag("input", input));

            if (this->prefix_.empty()) {
                /* almost meretricious to include input here,
                 * when called from scan2() it can only be whitespace
                 */
                return result_type::make_whitespace(input);
            } else {
                auto retval = assemble_final_token(span_type::from_string(prefix_));

                this->prefix_.clear();

                return retval;
            }
        } /*notify_eof*/
    } /*namespace scm*/
} /*namespace xo*/

/* end tokenizer.hpp */
