/** @file exprtype.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include <ostream>
//#include <cstdint>

namespace xo {
    namespace ast {
        /** @enum exprtype
         *  @brief enum to identify subclasses of xo::ast::Expression.
         *
         **/
        enum class exprtype {
            /** sentinel value **/
            invalid = -1,

            /** literal constant.  must satisfy both standard_layout_type + trivial **/
            constant,
            /** a literal constant that refers to a linkable named function **/
            primitive,
            /** variable/function definition **/
            define,
            /** variable assignment **/
            assign,
            /** function call **/
            apply,
            /** function definition **/
            lambda,
            /** variable reference **/
            variable,
            /** if-then-else **/
            ifexpr,
            /** sequence **/
            sequence,
            /** type conversion **/
            convert,

            /** not an expression.  comes last, counts entries **/
            n_expr
        };

        inline const char *
        expr2str(exprtype x)
        {
            switch(x) {
            case exprtype::invalid: return "?exprtype";
            case exprtype::constant: return "constant";
            case exprtype::primitive: return "primitive";
            case exprtype::define: return "define";
            case exprtype::assign: return "assign";
            case exprtype::apply: return "apply";
            case exprtype::lambda: return "lambda";
            case exprtype::variable: return "variable";
            case exprtype::ifexpr: return "if_expr";
            case exprtype::sequence: return "sequence";
            case exprtype::convert: return "convert";
            default: break;
            }

            return "???exprtype???";
        }

        /** @brief number of built-in expression types,  repr convenient for array sizing **/
        static constexpr std::size_t n_exprtype = static_cast<std::size_t>(exprtype::n_expr);

        inline std::ostream &
        operator<<(std::ostream & os,
                   exprtype x)
        {
            os << expr2str(x);
            return os;
        }
    } /*namespace ast*/
} /*namespace xo*/


/** end exprtype.hpp **/
