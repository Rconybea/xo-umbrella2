/* file parser_result.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "xo/expression/Expression.hpp"

namespace xo {
    namespace scm {
        enum parser_result_type {
            /** no result yet (incomplete expression) **/
            none,
            /** emit expression **/
            expression,
            /** emit error **/
            error
        };

        struct parser_result {
            using Expression = xo::ast::Expression;

        public:
            parser_result() = default;
            parser_result(parser_result_type type, rp<Expression> expr, const char * src_fn, std::string errmsg);

            static parser_result none();
            static parser_result error(const char * error_src_function,
                                       std::string errmsg);
            static parser_result expression(rp<Expression> expr);

            bool is_none() const { return result_type_ == parser_result_type::none; }
            bool is_expression() const { return result_type_ == parser_result_type::expression; }
            bool is_error() const { return result_type_ == parser_result_type::error; }

            parser_result_type result_type() const { return result_type_; }
            const rp<Expression> & result_expr() const { return result_expr_; }
            const char * error_src_function() const { return error_src_function_; }
            const std::string & error_description() const { return error_description_; }

        public:
            /** none|expression|error_description
             *
             *  @text
             *   result_type | error_src_function | error_description
             *  -------------+--------------------+-------------------
             *          none |            nullptr |             empty
             *    expression |            nullptr |             empty
             *         error |           non-null |         non-empty
             *  @endtext
             **/
            parser_result_type result_type_ = parser_result_type::none;
            /** if @ref result_state is parser_result_state::expression -> non-null result expression **/
            rp<Expression> result_expr_;
            /** if @ref result_state is parser_result_state::error -> non-null source function **/
            const char * error_src_function_ = nullptr;
            /** if @ref result_state is parser_result_state::error -> non-empty error description **/
            std::string error_description_;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end parser_result.hpp */
