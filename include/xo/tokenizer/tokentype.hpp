/** @file tokentype.hpp
 *
 *  author: Roland Conybeare, Jul 2024
 **/

#pragma once

#include "xo/indentlog/print/tag.hpp" // for STRINGIFY
#include <ostream>

namespace xo {
    namespace scm {
        /** @enum tokentype
         *  @brief enum to identify different schematica input token types
         *
         *  Schematica code examples:
         *
         *    type point :: { xcoord : f64, ycoord: f64 };
         *    type matrix :: array<double, 2>;  // 2-d array
         *
         *    decl hypot(x : f64, y : f64) -> f64;
         *
         *    def hypot(x : f64, y : f64) {
         *      let
         *        x2 = (x * x);
         *        y2 = (y * y);
         *        hypot2 = (x2 + y2);
         *      in
         *        sqrt(hypot2);
         *    };
         *
         *    def someconst 4;
         *
         *    def foo(v : vec<i32>) {
         *      def (pi : f64) = 3.1415926;
         *      def (h : (f64,f64) -> f64) = hypot;
         *
         *      h = hypot3;
         *    };
         *
         *    def matrixproduct(x : matrix, y : matrix) {
         *      [i,j : x.row(i) * y.col(j)];
         *    };
         **/
        enum class tokentype {
            /** sentinel value **/
            tk_invalid = -1,

            /** an integer constant (signed 64-bit integer) **/
            tk_i64,

            /** a 64-bit floating-point constant **/
            tk_f64,

            /** a string literal **/
            tk_string,

            /** a symbol **/
            tk_symbol,

            /** left-hand parenthesis '(' **/
            tk_leftparen,

            /** right-hand parenthesis ')' **/
            tk_rightparen,

            /** left-hand bracket '[' **/
            tk_leftbracket,

            /** right-hand bracket ']' **/
            tk_rightbracket,

            /** left-hand brace '{' **/
            tk_leftbrace,

            /** right-hand brace '}' **/
            tk_rightbrace,

            /** left-hand angle bracket '<' **/
            tk_leftangle,

            /** right-hand angle bracket '>' **/
            tk_rightangle,

            /** dot '.' **/
            tk_dot,

            /** comma ',' **/
            tk_comma,

            /** colon ':' **/
            tk_colon,

            /** double-colon '::' **/
            tk_doublecolon,

            /** semi-colon ';' **/
            tk_semicolon,

            /** '=' **/
            tk_singleassign,

            /** ':=' **/
            tk_assign,

            /** '->' **/
            tk_yields,

            /** keyworkd 'type' **/
            tk_type,

            /** keyword 'def' **/
            tk_def,

            /** keyword 'lambda' **/
            tk_lambda,

            /** keyword 'if' **/
            tk_if,

            /** keyword 'let' **/
            tk_let,

            /** keyword 'in' **/
            tk_in,

            /** keyword 'end' **/
            tk_end,

            n_tokentype /* comes last, counts #of entries */
        }; /*tokentype*/

        extern char const *
        tokentype_descr(tokentype tk_type);

        inline std::ostream &
        operator<< (std::ostream & os, tokentype tk_type) {
            os << tokentype_descr(tk_type);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end tokentype.hpp */
