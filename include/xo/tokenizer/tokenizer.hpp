/* file tokenizer.hpp
 *
 * author: Roland Conybeare, Jul 2024
 */

#pragma once

#include "token.hpp"
#include "span.hpp"
#include "xo/indentlog/scope.hpp"
#include <cassert>

namespace xo {
    namespace scm {
        /**
         *  Use:
         *  @code
         *    using tokenizer_type = tokenizer<char>;
         *    using span_type = tokenizer_type::span_type;
         *
         *    tokenizer_type tkz;
         *    span_type input = ...;
         *
         *    while !input.empty() {
         *        auto res = tkz.assemble_scan(input);
         *        const auto & tk = res.first;
         *
         *        // do something with tk if tk.is_valid()
         *
         *        input = input.after_prefix(res.second);
         *    }
         *
         *    if endofinput {
         *        auto tk = tzk.notify_eof()
         *
         *        // do something with tk if tk.is_valid()
         *    }
         *
         *    // expect !tkz.has_prefix()
         *
         *  @endcode
         **/
        template <typename CharT>
        class tokenizer {
        public:
            using token_type = token<CharT>;
            using span_type = span<const CharT>;
            using scan_result = std::pair<token_type, span_type>;

        public:
            tokenizer() = default;

            /** identifies whitespace chars.
             *  These are chars that do not belong to any token.
             *  They are not permitted to appear within
             *  a symbol or string token.
             *  Appearance of a whitespace char forces completion of
             *  preceding token.
             **/
            bool is_whitespace(CharT ch) const;

            /** identifies punctuation chars.
             *  These are chars that are not permitted to appear within
             *  a string/symbol token.  Instead they force completion of
             *  a preceding token,  and start a new token with themselves
             **/
            bool is_punctuation(CharT ch) const;

            /** true if tokenizer contains stored prefix of
             *  possibly-incomplete token
             **/
            bool has_prefix() const { !prefix_.empty(); }

            /** assemble token from text @p token_text
             **/
            token_type assemble_token(const span_type & token_text) const;

            /** scan for next input token,  given @p input **/
            scan_result scan(const span_type & input);

            /** notify end of input,  resolve any stored input **/
            token_type notify_eof();

        private:
            /** Accumulate partial token here.
             *  This will happen if input sent to @ref tokenizer::scan
             *  ends without a determinate token boundary.
             **/
            std::string prefix_;
        }; /*tokenizer*/

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
        tokenizer<CharT>::is_punctuation(CharT ch) const {
            switch(ch) {
            case '<':
                return true;
            case '>':
                return true;
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
                return true;
            case '=':
                return true;
            case '-':
                /* can't be punctuation -- can appear inside f64 token */
                return false;
            case '+':
                /* can't be punctuation -- can appear inside f64 token */
                return false;
            case '.':
                /* can't be punctuation -- can appear inside f64 token */
                return false;
            }

            return false;
        }

        template <typename CharT>
        auto
        tokenizer<CharT>::assemble_token(const span_type & token_text) const -> token_type
        {
            constexpr bool c_debug_flag = true;

            /* literal|pretty|streamlined */
            log_config::style = function_style::streamlined;

            scope log(XO_DEBUG(c_debug_flag));
            log && log(xtag("token_text", token_text));

            tokentype tk_type = tokentype::tk_invalid;
            std::string tk_text;

            const CharT * tk_start = token_text.lo();
            const CharT * tk_end = token_text.hi();

            const CharT * ix = tk_start;

            /* switch here applies to the first character in a token */
            switch (*ix) {
            case '-':
            case '+':
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

                /* token will be one of: {i64, f64, dot}: */
                for(; ix != token_text.hi(); ++ix) {
                    if((*ix == '-') || (*ix == '+')) {
                        /* sign allowed:
                         * 1. before period and before first digit
                         * 2. after exponent
                         */
                        if (!period_flag && !number_flag && !sign_flag) {
                            sign_flag = true;
                        } else if (exponent_flag && !exponent_digit_flag) {
                            exponent_sign_flag = true;
                        } else {
                            throw std::runtime_error
                                (tostr("tokenizer::assemble_token",
                                       ": improperly placed sign indicator",
                                       xtag("pos", ix - tk_start),
                                       xtag("char", *ix)));
                        }
                    } else if(*ix == '.') {
                        if (period_flag) {
                            throw (std::runtime_error
                                   (tostr("tokenizer::assemble_token",
                                          ": duplicate decimal point",
                                          xtag("pos", ix - tk_start),
                                          xtag("char", *ix))));
                        }

                        period_flag = true;
                    } else if((*ix == 'e') || (*ix == 'E')) {
                        if (exponent_flag) {
                            throw (std::runtime_error
                                   (tostr("tokenizer::assemble_token",
                                          ": duplicate exponent marker",
                                          xtag("pos", ix - tk_start),
                                          xtag("char", *ix))));
                        }

                        exponent_flag = true;
                    } else if(isdigit(*ix)) {
                        if (exponent_flag) {
                            /* need digit before exponent to recognize as number */
                            exponent_digit_flag = true;
                        } else {
                            number_flag = true;
                        }
                    } else {
                        /* invalid input */
                        throw (std::runtime_error
                               (tostr("tokenizer::assemble_token",
                                      ": unexpected character in numeric constant",
                                      xtag("pos", ix - tk_start),
                                      xtag("char", *ix))));
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

                break;
            }
            case '"':
            {
                log && log("recognize string-token");

                tk_type = tokentype::tk_string;

                tk_text.reserve(token_text.hi() - token_text.lo());

                ++ix; /*skip initial " char*/

                for (; ix != token_text.hi(); ++ix) {
                    log && log(xtag("*ix", *ix));

                    bool endofstring = false;

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
                            throw std::runtime_error
                                (tostr("tokenizer::assemble_token",
                                       ": malformed string literal",
                                       xtag("input", std::string_view(token_text.lo(),
                                                                      token_text.hi()))));
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
                            throw std::runtime_error
                                (tostr("tokenizer::assemble_token",
                                       ": unexpected \\-escaped char",
                                       xtag("char", *ix)));
                        }
                        break;
                    default:
                        tk_text.push_back(*ix);
                        break;
                    }

                    if (endofstring)
                        break;
                }

                if (ix != token_text.hi()) {
                    throw std::runtime_error
                        (tostr("tokenizer::assemble_token",
                               ": expected \" to end string literal",
                               xtag("input", std::string_view(token_text.lo(),
                                                              token_text.hi()))));
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
                tk_type = tokentype::tk_colon;
                ++ix;
                break;
            case '=':
                tk_type = tokentype::tk_singleassign;
                ++ix;
                break;
            default:
                break;
            }

            if (tk_type == tokentype::tk_invalid) {
                throw std::runtime_error(tostr("tokenizer::assemble_token",
                                               ": unexpected input x",
                                               xtag("x", *ix)));
            }

            if ((tk_type == tokentype::tk_i64)
                || (tk_type == tokentype::tk_f64)
                || (tk_type == tokentype::tk_symbol))
            {
                /* re-parse in token::i64_value() / token::f64_value() */
                tk_text = std::string(tk_start, tk_end);
            } else if (tk_type == tokentype::tk_string) {
                ; /* nothing to do here -- desired tk_text already constructed */
            }

            return token_type(tk_type, std::move(tk_text));
        } /*assemble_token*/

        template <typename CharT>
        auto
        tokenizer<CharT>::scan(const span_type & input) -> scan_result
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("input", input));

            const CharT * ix = input.lo();

            /* skip whitespace */
            while (is_whitespace(*ix) && (ix != input.hi()))
                ++ix;

            if(ix == input.hi()) {
                /* no-op */
                return {
                    token_type::invalid(),
                    input.prefix(ix)
                };
            }

            /* here: *ix is not whitespace */

            auto whitespace = input.prefix(ix);

            log && log(xtag("whitespace.size", whitespace.size()));

            /* tk_start points to beginning of token
             * (after any whitespace)
             */
            const CharT * tk_start = ix;

            if (is_punctuation(*ix)) {
                /* 1-character token */
                ++ix;
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
                        throw std::runtime_error
                            (tostr("tokenizer::scan",
                                   ": must use \\n or \\r to encode newline/cr in"
                                   " string literal"));
                    }

                    prev_ch = *ix;
                }

                if (!complete_flag) {
                    /* need more input to know if/when tokken complete */
                    this->prefix_ += std::string(tk_start, input.hi());

                    log && log(xtag("captured-prefix", this->prefix_));
                }
            } else {
                /* scan until:
                 * - whitespace
                 * - punctuation
                 */
                for (; ix != input.hi(); ++ix) {
                    if (is_whitespace(*ix) || is_punctuation(*ix))
                        break;
                }

                if (ix == input.hi()) {
                    /* need more input to know if/when token complete */
                    this->prefix_ += std::string(tk_start, input.hi());

                    log && log(xtag("captured-prefix", this->prefix_));
                }
            }

            auto token_span = input.after_prefix(whitespace).prefix(ix);

            token tk
                = (this->prefix_.empty()
                   ? assemble_token(token_span)
                   : token_type(tokentype::tk_invalid));

            return scan_result
                { tk, input.prefix(whitespace.size() + token_span.size()) };
        } /*scan*/

        template <typename CharT>
        auto
        tokenizer<CharT>::notify_eof() -> token_type {
            constexpr bool c_debug_flag = true;

            scope log(XO_DEBUG(c_debug_flag));

            token tk
                = (this->prefix_.empty()
                   ? token_type(tokentype::tk_invalid)
                   : assemble_token(span_type(&prefix_[0],
                                              &prefix_[prefix_.size()])));

            this->prefix_.clear();

            return tk;
        } /*notify_eof*/
    } /*namespace scm*/
} /*namespace xo*/

/* end tokenizer.hpp */
