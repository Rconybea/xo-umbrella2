/* file tokenizer_error.hpp
 *
 * author: Roland Conybeare, Jun 2025
 */

#pragma once

#include "tokentype.hpp"
#include "span.hpp"
#include <iomanip>

namespace xo {
    namespace scm {
        /** @class tokenizer_error
         *  @brief represent a lexing error, with context
         *
         *  @tparam CharT  representation for single characters
         **/
        template <typename CharT>
        class tokenizer_error {
        public:
            using span_type = span<const CharT>;

        public:
            /** @defgroup tokenizer-error-ctors **/
            ///@{

            /** Default ctor represent a not-an-error sentinel object **/
            tokenizer_error() = default;
            /** Constructor to capture parsing error context
             *  @p tk_start   current position on entry to scanner
             *  @p whitespace number of chars initial whitespace
             *  @p error_pos  error location relative to token start
             **/
            tokenizer_error(const char * src_function,
                            const char * error_description,
                            span_type input_line,
                            size_t tk_start,
                            size_t whitespace,
                            size_t error_pos)
                : src_function_{src_function},
                  error_description_{error_description},
                  input_line_{input_line},
                  tk_entry_{tk_start},
                  whitespace_{whitespace},
                  error_pos_{error_pos} {}
            ///@}

            /** @defgroup tokenizer-error-access-methods **/
            ///@{

            const char * src_function() const { return src_function_; }
            const char * error_description() const { return error_description_; }
            const span_type& input_line() const { return input_line_; }
            size_t tk_start() const { return tk_entry_; }
            size_t whitespace() const { return whitespace_; }
            size_t error_pos() const { return error_pos_; }

            ///@}

            /** @defgroup tokenizer-error-general-methods **/
            ///@{

            /** true, except for a sentinel error object **/
            bool is_error() const { return error_description_ != nullptr; }
            /** true except for object in sentinel state **/
            bool is_not_an_error() const { return error_description_ == nullptr; }

            /** Print representation to stream @p os. Intended for tokenizer diagnostics.
             *  For Schematika errors prefer @ref report
             **/
            void print(std::ostream & os) const;

            /** Print human-oriented error report on @p os. **/
            void report(std::ostream & os) const;

            ///@}

        private:
            /** @defgroup tokenizer-error-instance-vars **/
            ///@{

            /** source location (in tokenizer) at which error identified **/
            char const * src_function_ = nullptr;
            /** static error description **/
            char const * error_description_ = nullptr;
            /** complete current input line (to the extent captured)
             *  that contains error
             **/
            span_type input_line_ = span_type::make_null();
            /** position (relative to line_.lo) of token start where error encountered **/
            size_t tk_entry_ = 0;
            /** number of characters of initial whitespace skipped before token start **/
            size_t whitespace_ = 0;
            /** position (relative to @ref tk_entry_) of error **/
            size_t error_pos_ = 0;

            ///@}
        }; /*error_token*/

        template <typename CharT>
        void
        tokenizer_error<CharT>::print(std::ostream & os) const {
            os << "<tokenizer-error"
               << xtag("src-function", src_function_)
               << xtag("message", error_description_)
               << xtag("input", input_line_)
               << xtag("whitespace", whitespace_)
               << xtag("tk-start", tk_entry_)
               << xtag("error-pos", error_pos_)
               << ">";
        }

        template <typename CharT>
        void
        tokenizer_error<CharT>::report(std::ostream & os) const {
            using namespace std;

            if (error_description_) {
                const char * prefix = "input: ";
                const size_t tk_indent = strlen(prefix) + tk_entry_ + whitespace_;
                //const size_t msg_length = strlen(error_description_);

                const size_t error_pos = 1 + tk_entry_ + whitespace_ + error_pos_;

                os << "char: " << error_pos << endl;
                os << prefix;
                for (const char *p = input_line_.lo(), *e = input_line_.hi(); p < e; ++p)
                    os << *p;
                os << endl;
                os << std::setw(tk_indent) << " ";

                for (size_t i = 0; i < error_pos_; ++i) {
                    os << '_';
                }
                os << '^' << endl;

                os << error_description_ << endl;
            }
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
