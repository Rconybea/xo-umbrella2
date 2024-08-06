/* file token.hpp
 *
 * author: Roland Conybeare, Jul 2024
 */

#pragma once

#include "tokentype.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <stdexcept>
#include <ostream>
#include <string>
#include <cstdint>

namespace xo {
    namespace scm {
        namespace detail {
            /* compute a * b^p,  p >= 0 */
            constexpr double
            pow_aux(double a, double b, int p) {
                while (p > 0) {
                    if (p % 2 == 1) {
                        /* a * b^p = a * b^(2q + 1) = a.b * 10^(2q) */
                        a *= b;
                        p -= 1;
                    } else {
                        /* a * b^p = a * b^(2q) = a * (b^2)^q */
                        b = b * b;
                        p /= 2;
                    }
                }

                /* a * b^0 = a */
                return a;
            }

            constexpr double
            pow10(int p) {
                if (p >= 0)
                    return pow_aux(1.0, 10.0, p);
                else
                    return 1.0 / pow_aux(1.0, 10.0, -p);
            }
        }

        template <typename CharT>
        class token {
        public:
            token() = default;
            token(tokentype tk_type, const std::string & text = "")
                : tk_type_{tk_type}, text_{text} {}

            static token invalid() { return token(); }
            static token i64_token(const std::string & txt) {
                return token(tokentype::tk_i64, txt);
            }
            static token f64_token(const std::string & txt) {
                return token(tokentype::tk_f64, txt);
            }
            static token string_token(const std::string & txt) {
                return token(tokentype::tk_string, txt);
            }
            static token symbol_token(const std::string & txt) {
                return token(tokentype::tk_symbol, txt);
            }
            static token leftangle() { return token(tokentype::tk_leftangle); }
            static token rightangle() { return token(tokentype::tk_rightangle); }
            static token leftparen() { return token(tokentype::tk_leftparen); }
            static token rightparen() { return token(tokentype::tk_rightparen); }
            static token leftbracket() { return token(tokentype::tk_leftbracket); }
            static token rightbracket() { return token(tokentype::tk_rightbracket); }
            static token leftbrace() { return token(tokentype::tk_leftbrace); }
            static token rightbrace() { return token(tokentype::tk_rightbrace); }
            static token dot() { return token(tokentype::tk_dot); }
            static token comma() { return token(tokentype::tk_comma); }
            static token colon() { return token(tokentype::tk_colon); }
            static token doublecolon() { return token(tokentype::tk_doublecolon); }
            static token semicolon() { return token(tokentype::tk_semicolon); }
            static token singleassign() { return token(tokentype::tk_singleassign); }
            static token assign() { return token(tokentype::tk_assign); }
            static token yields() { return token(tokentype::tk_yields); }

            static token type() { return token(tokentype::tk_type); }
            static token def() { return token(tokentype::tk_def); }
            static token lambda() { return token(tokentype::tk_lambda); }
            static token if_token() { return token(tokentype::tk_if); }
            static token let() { return token(tokentype::tk_let); }
            static token in() { return token(tokentype::tk_in); }
            static token end() { return token(tokentype::tk_end); }

            tokentype tk_type() const { return tk_type_; }
            const std::string & text() const { return text_; }

            bool is_valid() const { return tk_type_ != tokentype::tk_invalid; }
            bool is_invalid() const { return tk_type_ == tokentype::tk_invalid; }

            /** expect input matching
             *    [+|-][0-9][0-9]*
             **/
            std::int64_t i64_value() const;
            /** expect input matching
             *    [+|-][0-9]*[.][0-9]*[e|E][+|-][0-9]*
             **/
            double f64_value() const;

            /** print human-readable token representation on stream @p os **/
            void print(std::ostream & os) const;

        private:
            /** category for this token **/
            tokentype tk_type_ = tokentype::tk_invalid;

            /** characters comprising this token.
             *  only provided for certain token types:
             *
             *    tk_i64
             *    tk_f64
             *    tk_string
             *    tk_symbol
             **/
            std::string text_;
        }; /*token*/

        template <typename CharT>
        std::int64_t
        token<CharT>::i64_value() const {
            if (tk_type_ != tokentype::tk_i64) {
                throw (std::runtime_error
                       (tostr("token::i64_value",
                              ": token with type tk found where tk_i64 expected",
                              xtag("tk", tk_type_))));
            }

            if (text_.empty()) {
                throw (std::runtime_error
                       (tostr("token::i64_value",
                              ": unexpected empty input string for tk_i64 token")));
            }

            int sign = 1;
            int value = 0;
            {
                auto ix = text_.begin();
                auto end_ix = text_.end();

                CharT ch = *ix;

                if (ch == '+') {
                    ++ix;
                } else if (ch == '-') {
                    sign = -1;
                    ++ix;
                }

                if (ix == end_ix) {
                    throw (std::runtime_error
                           (tostr("token::i64_value",
                                  ": input text found where at least one digit expected",
                                  xtag("text", text_))));
                }

                for (; ix != end_ix; ++ix) {
                    CharT ch = *ix;

                    if ((ch >= '0') && (ch <= '9')) {
                        value *= 10;
                        value += (ch - '0');
                    } else {
                        throw (std::runtime_error
                               (tostr("token::i64_value",
                                      ": unexpected char ch in integer token",
                                      xtag("ch", ch))));
                    }
                }
            }

            return sign * value;
        } /*i64_value*/

        template <typename CharT>
        double
        token<CharT>::f64_value() const {
            if (tk_type_ != tokentype::tk_f64) {
                throw (std::runtime_error
                       (tostr("token::f64_value",
                              ": token with type tk found where tk_f64 expected",
                              xtag("tk", tk_type_))));
            }

            if (text_.empty()) {
                throw (std::runtime_error
                       (tostr("token::f64_value",
                              ": unexpected empty input string for tk_f64 token")));
            }

            int sign = 1;
            /* integer representing denormalized unsigned mantissa
             * (mantissa scaled by smallest power of 10 sufficient to make
             *  it an integer)
             */
            std::int64_t mantissa = 0;
            /* counts #of digits to the right of decimal point '.' */
            int rh_digits = 0;
            /* sign of exponent */
            int exp_sign = 1;
            /* value of exponenct = integer to the right of 'e' or 'E' */
            int exponent = 0;

            /* floating-point value will represent
             *   sign * mantissa * 10^(sign*exponent - rh_digits)
             */
            {
                auto ix = text_.begin();
                auto end_ix = text_.end();

                CharT ch = *ix;

                if (ch == '+') {
                    ++ix;
                } else if (ch == '-') {
                    sign = -1;
                    ++ix;
                }

                if (ix == end_ix) {
                    throw (std::runtime_error
                           (tostr("token::f64_value",
                                  ": input text found where at least one digit expected",
                                  xtag("text", text_))));
                }

                /* true iff decimal point '.' present in mantissa */
                bool have_decimal_point = false;
                /* true iff exponent prefix 'e' or 'E' present */
                //bool have_exponent = false;
                /* counts number of digits in mantissa
                 * (both before and after, but not including, any decimal point
                 */
                int m_digits = 0;
                /* digits to the left of decimal point */
                int lh_digits = 0;

                /* loop over mantissa digits */
                for (; ix != end_ix; ++ix) {
                    CharT ch = *ix;

                    if (ch == '.') {
                        if (have_decimal_point) {
                            throw (std::runtime_error
                                   (tostr("token::f64_value",
                                          ": input text found where at most one decimal point expected",
                                          xtag("text", text_))));
                        }

                        have_decimal_point = true;
                        lh_digits = m_digits;
                    } else if ((ch >= '0') && (ch <= '9')) {
                        mantissa *= 10;
                        mantissa += (ch - '0');
                        ++m_digits;
                    } else if (ch == 'e' || ch == 'E') {
                        //have_exponent = true;
                        break; // done with mantissa
                    } else {
                        throw (std::runtime_error
                               (tostr("token::i64_value",
                                      ": unexpected char ch in integer token",
                                      xtag("ch", ch))));
                    }
                }

                if (have_decimal_point)
                    rh_digits = m_digits - lh_digits;

                if (ix != end_ix) {
                    /* continue to read exponent */

                    /* skip e|E */
                    ++ix;

                    if (ix == end_ix) {
                        throw (std::runtime_error
                               (tostr("token::f64_value",
                                      ": on input text, expect at least one digit following exponent marker e|E",
                                      xtag("text", text_))));
                    }

                    CharT ch = *ix;

                    if (ch == '+') {
                        ++ix; /*skip*/
                    } else if (ch == '-') {
                        exp_sign = -1;
                        ++ix;
                    }

                    for (; ix != end_ix; ++ix) {
                        CharT ch = *ix;

                        if ((ch >= '0') && (ch <= '9')) {
                            exponent *= 10;
                            exponent += (ch - '0');
                        } else {
                            throw (std::runtime_error
                                   (tostr("token::f64_value",
                                          "; on input text, expect only digits following"
                                          " (possibly signed) exponenct marker",
                                          xtag("text", text_))));
                        }
                    }
                }
            }

            /* floating-point value will represent
             *   sign * mantissa * 10^(sign*exponent - rh_digits)
             */

            double mantissa_f64 = sign * mantissa;

#ifdef OBSOLETE_DEBUG
            std::cerr << xtag("text", text_)
                      << xtag("rh_digits", rh_digits)
                      << xtag("mantissa_f64", mantissa_f64)
                      << xtag("exp_sign", exp_sign)
                      << xtag("exponent", exponent)
                      << std::endl;
#endif

            double retval = (mantissa_f64
                             * detail::pow10((exp_sign * exponent)
                                             - rh_digits));

            return retval;
        } /*f64_value*/

        template <typename CharT>
        void
        token<CharT>::print(std::ostream & os) const {
            os << "<token"
               << xtag("type", tk_type_)
               << xtag("text", text_)
               << ">";
        } /*print*/

        template <typename CharT>
        inline std::ostream &
        operator<< (std::ostream & os,
                    const token<CharT> & tk)
        {
            tk.print(os);
            return os;
        }
    } /*Namespace scm*/
} /*namespace xo*/


/* end token.hpp */
