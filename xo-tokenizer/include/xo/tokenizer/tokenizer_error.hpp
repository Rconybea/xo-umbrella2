/* file tokenizer_error.hpp
 *
 * author: Roland Conybeare, Jun 2025
 */

#pragma once

#include "input_state.hpp"
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
            using input_state_type = input_state<CharT>;
            using span_type = span<const CharT>;

        public:
            /** @defgroup tokenizer-error-ctors **/
            ///@{

            /** Default ctor represents a not-an-error sentinel object **/
            tokenizer_error() = default;
            /** Constructor to capture parsing error context
             *  @p tk_start   current position on entry to scanner
             *  @p error_pos  error location relative to token start
             **/
            tokenizer_error(const char * src_function,
                            const char * error_description,
                            const input_state_type & input_state,
                            size_t error_pos)
                : src_function_{src_function},
                  error_description_{error_description},
                  input_state_{input_state},
                  error_pos_{error_pos}
                {
                    scope log(XO_DEBUG(input_state.debug_flag()));

                    log && log(xtag("input_state.current_pos", input_state.current_pos()),
                               xtag("error_pos", error_pos));
                }
            ///@}

            /** @defgroup tokenizer-error-access-methods **/
            ///@{

            const char * src_function() const { return src_function_; }
            const char * error_description() const { return error_description_; }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning"
            const input_state_type & input_state() const { return input_state_; }
#pragma GCC diagnostic pop
            size_t tk_start() const { return input_state_.current_pos(); }
            size_t whitespace() const { return input_state_.whitespace(); }
            size_t error_pos() const { return error_pos_; }

            ///@}

            /** @defgroup tokenizer-error-general-methods **/
            ///@{

            /** true, except for a sentinel error object **/
            bool is_error() const { return error_description_ != nullptr; }
            /** false except for object in sentinel state **/
            bool is_not_an_error() const { return error_description_ == nullptr; }

            /** Print representation to stream @p os. Intended for tokenizer diagnostics.
             *  For Schematika errors prefer @ref report
             **/
            void print(std::ostream & os) const;

            /** Print human-oriented error report on @p os. **/
            void report(std::ostream & os) const;

            ///@}

        private:
            /** @defgroup tokenizer-error-vars **/
            ///@{

            /** source location (in tokenizer) at which error identified **/
            char const * src_function_ = nullptr;
            /** static error description **/
            char const * error_description_ = nullptr;
            /** input state associated with this error.
             *  Sufficient to precisely locate it with context.
             **/
            input_state_type input_state_;
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
               << xtag("input", input_state_.current_line())
               << xtag("whitespace", input_state_.whitespace())
               << xtag("error-pos", error_pos_)
               << ">";
        }

        template <typename CharT>
        void
        tokenizer_error<CharT>::report(std::ostream & os) const {
            using namespace std;

            if (error_description_) {
                const char * prefix = "input: ";
                /* input_state.current_pos: position of first character following preceding token.
                 * input_state.whitespace:  whitespace between current_pos and start of failing token
                 * error_pos:               position (relative to start) at which failure detected
                 */
                const size_t tk_start = input_state_.current_pos() + input_state_.whitespace();
                const size_t tk_indent = (strlen(prefix) + tk_start);
                const size_t error_pos = 1 + tk_start + error_pos_;

                os << "char: " << error_pos << endl;
                os << prefix;
                for (const char *p = input_state_.current_line().lo(),
                         *e = input_state_.current_line().hi(); p < e; ++p)
                {
                    os << *p;
                }
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
