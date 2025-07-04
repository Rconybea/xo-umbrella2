/* file parser.hpp
 *
 * author: Roland Conybeare, Jul 2024
 */

#pragma once

#include "exprstatestack.hpp"
#include "envframestack.hpp"
#include <stdexcept>

namespace xo {
    namespace scm {
        /** schematica parser
         *
         *  Examples:
         *
         *    decltype point
         *
         *    // forward declarations
         *    decl pi : f64;
         *    decl fib(n : i32) -> i32;
         *
         *    def pi = 3.14159265;  // constant. = is single assignment
         *
         *    def fib(n : i32) -> i32 {
         *      // nested defs ok
         *      def aux(n : i32, s1 : i32, s2 : i32) -> i32 {
         *        // or:
         *        //   (n == 0) ? s1 : aux(n - 1, s1 + s2, s1)
         *        //
         *        if (n == 0) {
         *          s1;
         *        } else {
         *          aux(n - 1, s1 + s2, s1);
         *        }
         *
         *        // or:
         *        //  if (n == 0) ? s1 : aux(n - 1, s1 + s2, s1)
         *      }
         *
         *      aux(n=n, s1=1, s2=0);
         *    }
         *
         *    def x := "fu"; // non-constant
         *    x += "bar";
         *
         *    def anotherfib = lambda(n : i32) { fib(n) };
         *
         *    def any : object;
         *    def l : list<object> = '();
         *
         *    deftype point :: {x : f64, y : f64};
         *    deftype polar :: {arg : f64, mag : f64};
         *
         *    def polar2rect(pt : polar) -> point {
         *      point(x = pt.mag * cos(arg),
         *            y = pt.mag * sin(arg));
         *    }
         *
         * Grammar:
         *   toplevel-program = $toplevel-expression(1); ..; $toplevel-expression(n)
         *
         * if interactive:
         *   toplevel-expression = expression
         * else
         *   toplevel-expression = type-decl | define-expr
         *
         *   type-decl        = decltype $typename [<$tp1 .. $tpn>]
         *   expression       = type-decl
         *                       | define-expr
         *                       | literal-expr
         *                       | variable-expr
         *                       | apply-expr
         *                       | if-expr
         *                       | lambda-expr
         *                       | arithmetic-expr
         *                       | block
         *
         *   define-expr      = type-decl
         *                       | type-def
         *                       | variable-def
         *                       | function-decl
         *                       | function-def
         *
         *   type-def         = deftype $typename [<$tp1 .. $tpn>] :: type-def-rhs
         *   type-def-rhs     = object
         *                       | bool
         *                       | i128 | i64 | i32 | i16 | i8
         *                       | f128 | f64 | f32 | f16
         *                       | struct $typename { ($membername(i) : $typename(i))* }
         *                          [end $typename]
         *                       | tuple $typename { $typename(1), .., $typename(n) }
         *                          [end $typename]
         *                       | copytype $typename
         *                       | subtype $typename { ($member(i) : $typename(i))* }
         *
         *   variable-def     = decl $varname [: $typename] [= expression]
         *   function-decl    = decl $functionname($varname(1) : $typename(1),
         *                                         ..,
         *                                         $varname(n) : $typename(n)) -> $typename[ret]
         *   function-def     = def $functionname($varname(1) : $typename(1),
         *                                        ..,
         *                                        $varname(n) : $typename(n)) [-> $typename[ret]]
         *                       body-expr
         *                       [ end $functionname ]
         *   literal-expr     = integer-literal
         *                       | fp-literal
         *                       | string-literal
         *                       | symbol-literal
         *                       | struct-literal
         *
         *   variable-expr    = $varname
         *   apply-expr       = fn-expr(arg-expr(1), .., arg-expr(n))
         *     fn-expr          = expression
         *     arg-expr(i)      = expression
         *
         *   if-expr          = if (test-expr) then-block else else-block
         *                       | ((test-expr) ? then-expr : else-expr)
         *     test-expr        = expression
         *     then-block       = block
         *     else-block       = block
         *
         *   block            = { (definition | expression)* }
         *
         *   lambda-expr      = lambda ($paramname(1) : $type(1),
         *                              ..,
         *                              $paramname(n) : $type(n)) body-expr
         *    body-expr       = expression
         *
         *   arithmetic-expr  = expression binop expression
         *
         *    binop           = +
         *                       | -
         *                       | *
         *                       | /
         *                       | |
         *                       | &
         *                       | ^
         *                       | ==
         *                       | !=
         *                       | <
         *                       | <=
         *                       | =>
         *                       | >
         *
         **/
        class parser {
        public:
            using Expression = xo::ast::Expression;
            using token_type = exprstate::token_type; // token<char>;

        public:
            /** create parser in initial state;
             *  parser is ready to receive tokens via @ref include_token
             **/
            parser() = default;

            /** for diagnostics: number of entries in parser stack **/
            std::size_t stack_size() const { return xs_stack_.size(); }
            /** for diagnostics: exprstatetype at level @p i
             *  (taken relative to top of stack)
             *
             *  @pre 0 <= i < stack_size
             **/
            exprstatetype i_exstype(std::size_t i) const {
                std::size_t z = xs_stack_.size();

                if (i < z) {
                    return xs_stack_[i]->exs_type();
                }

                /* out of bounds */
                return exprstatetype::invalid;
            }

            exprstate const * i_exstate(std::size_t i) const {
                std::size_t z = xs_stack_.size();

                if (i < z) {
                    return xs_stack_[i].get();
                }

                /* out of bounds */
                return nullptr;
            }

            /** true iff parser contains state for an incomplete expression.
             *  For this to be true,  parser must have consumed at least one token
             *  since end of last toplevel expression
             **/
            bool has_incomplete_expr() const;

            /** put parser into state for beginning an interactive session.
             **/
            void begin_interactive_session();

            /** put parser into state for beginning of a translation unit
             *  (i.e. input stream)
             **/
            void begin_translation_unit();

            /** include next token @p tk and increment parser state.
             *
             *  @param tk  next input token
             *  @return parsed expression, if @p tk completes an expression.
             *  otherwise nullptr
             **/
            rp<Expression> include_token(const token_type & tk);

            /** print human-readable representation on stream @p os **/
            void print(std::ostream & os) const;

        private:
            /** state recording state associated with enclosing expressions.
             *
             *  Note: at least asof c++23, the std::stack api doesn't support access
             *  to members other than the top.
             *
             *  for stack with N elements (N = stack_.size()):
             *  - bottom of stack is stack_[0]
             *  - top of stack is stack_[N-1]
             **/
            exprstatestack xs_stack_;

            /** environment frames for lexical context.
             *  push a frame on each nested lambda;
             *  pop when lambda body goes out of scope
             **/
            envframestack env_stack_;

        }; /*parser*/

        inline std::ostream &
        operator<< (std::ostream & os,
                    const parser & x) {
            x.print(os);
            return os;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end parser.hpp */
