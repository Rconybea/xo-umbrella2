/* reader_error.hpp
 *
 * Author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "xo/tokenizer/tokenizer_error.hpp"

namespace xo {
    namespace scm {
        class reader_error {
        public:
            using input_state_type = typename tokenizer_error<char>::input_state_type;

        public:
            /** default ctor represents a not-an-error sentinel object **/
            reader_error() = default;
            /** construct to capture parsing error context
             *  @
             **/
            reader_error(const char * src_function,
                         std::string error_description,
                         const input_state_type & input_state,
                         size_t error_pos)
                : tk_error_{src_function, error_description, input_state, error_pos}
                {}

            const tokenizer_error<char> & tk_error() const { return tk_error_; }

            /** true, except for sentinel not-an-error object **/
            bool is_error() const { return tk_error_.is_error(); }
            /** false, except for object in sentinel state **/
            bool is_not_an_error() const { return tk_error_.is_not_an_error(); }

            const char * src_function() const { return tk_error_.src_function(); }

            /** print error representation to stream @p os.  Intended for parser/tokenizer
             *  diagnostics.  For Schematika errors prefer @ref report
             **/
            void print(std::ostream & os) const { tk_error_.print(os); }

            /** print human-oriented error report on @p os. **/
            void report(std::ostream & os) const { tk_error_.report(os); }

        private:
            /** for parser-level errors, will still use this for
             *  {src function, error description, input state and error pos}
             **/
            tokenizer_error<char> tk_error_;
        };
    }
}

/* end reader_error.hpp */
