/* file scan_result.hpp
 *
 * author: Roland Conybeare, Jun 2025
 */

#pragma once

#include "token.hpp"
#include "tokenizer_error.hpp"

namespace xo {
    namespace scm {
        /** @brief Represent result of parsing one input token.
         *
         *  Possible outcomes fall into several categories
         *  (with T: @c token_.is_valid(), E: @cerror_.is_error())
         *
         *  | T     | E     | description       |
         *  |-------+-------+-------------------|
         *  | false | false | end of input      |
         *  | true  | false | parsed token in T |
         *  | false | true  | parse error in E  |
         *
         **/
        template <typename CharT>
        class scan_result {
        public:
            using token_type = token<CharT>;
            using span_type = span<const CharT>;
            using error_type = tokenizer_error<CharT>;

        public:
            scan_result(const token_type & token,
                        const span_type & consumed,
                        const error_type & error = error_type())
                : token_{token}, consumed_{consumed}, error_{error} {}

            static scan_result make_whitespace(const span_type & prefix_input);
            static scan_result make_partial(const span_type & prefix_input);

            bool is_eof_or_ambiguous() const { return token_.is_invalid() && error_.is_not_an_error(); }
            bool is_token() const { return token_.is_valid(); }
            bool is_error() const { return error_.is_error(); }

            const token_type & get_token() const { return token_; }
            const span_type & consumed() const { return consumed_; }
            const error_type & error() const { return error_; }

        public:
            /** successfully parsed token, whenever tk_type != tokentype::tk_invalid **/
            token_type token_;
            /** input span represented by .token, on success. Otherwise not defined **/
            span_type consumed_;
            /** error description, whenever .error_.is_error() is true **/
            error_type error_;
        };

        template <typename CharT>
        auto scan_result<CharT>::make_whitespace(const span_type& whitespace_input) -> scan_result
        {
            return scan_result(token_type::invalid(), whitespace_input /*consumed*/);
        }

        template <typename CharT>
        auto scan_result<CharT>::make_partial(const span_type& prefix_input) -> scan_result
        {
            return scan_result(token_type::invalid(), prefix_input /*consumed*/);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end scan_result.hpp */
