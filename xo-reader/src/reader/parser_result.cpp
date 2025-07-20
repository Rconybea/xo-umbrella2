/* file parser_result.cpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#include "parser_result.hpp"

namespace xo {
    namespace scm {
        parser_result::parser_result(parser_result_type type, rp<Expression> expr, const char * src_fn, std::string errmsg)
            : result_type_{type}, result_expr_{std::move(expr)}, error_src_function_{src_fn}, error_description_{std::move(errmsg)}
        {}

        parser_result
        parser_result::none()
        {
            return parser_result(parser_result_type::none,
                                 nullptr,
                                 nullptr,
                                 std::string());
        }

        parser_result
        parser_result::error(const char * error_src_function,
                             std::string errmsg)
        {
            return parser_result(parser_result_type::error,
                                 nullptr,
                                 error_src_function,
                                 std::move(errmsg));
        }

        parser_result
        parser_result::expression(rp<Expression> expr)
        {
            return parser_result(parser_result_type::expression,
                                 std::move(expr),
                                 nullptr /*src_function*/,
                                 std::string());
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end parser_result.cpp */
