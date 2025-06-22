/* file tokenizer_error.hpp
 *
 * author: Roland Conybeare, Jun 2025
 */

#pragma once

#include "tokentype.hpp"
#include "span.hpp"

namespace xo {
    namespace scm {
        /** represent a lexing error, with context **/
        template <typename CharT>
        class tokenizer_error {
        public:
            using span_type = span<const CharT>;

        public:
            /** @brief default ctor represent a not-an-error error object **/
            tokenizer_error() = default;
            tokenizer_error(char const * src_function,
                            char const* error_description,
                            span_type input_line, size_t error_pos)
                : src_function_{src_function},
                  error_description_{error_description},
                  input_line_{input_line},
                  error_pos_{error_pos} {}

            char const* src_function() const { return src_function_; }
            char const* error_description() const { return error_description_; }
            size_t error_pos() const { return error_pos_; }
            const span_type& input_line() const { return input_line_; }

            bool is_not_an_error() const { return error_description_ == nullptr; }
            bool is_error() const { return error_description_ != nullptr; }

            void print(std::ostream & os) const;

        private:
            /** source location (in tokenizer) at which error identified **/
            char const * src_function_ = nullptr;
            /** static error description **/
            char const * error_description_ = nullptr;
            /** position (relative to line_.lo) of error **/
            size_t error_pos_ = 0;
            /** complete input line (to the extent available)
             *  containing error
             **/
            span_type input_line_ = span_type::make_null();
        }; /*error_token*/

        template <typename CharT>
        void
        tokenizer_error<CharT>::print(std::ostream & os) const {
            os << "<tokenizer-error"
               << xtag("src-function", src_function_)
               << xtag("message", error_description_)
               << xtag("error-pos", error_pos_)
               << xtag("input", input_line_)
               << ">";
        }

        template <typename CharT>
        inline std::ostream &
        operator<< (std::ostream & os,
                    const tokenizer_error<CharT> & tkerr)
        {
            tkerr.print(os);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end tokenizer_error.hpp */
