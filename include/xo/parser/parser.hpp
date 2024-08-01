/* file parser.hpp
 *
 * author: Roland Conybeare, Jul 2024
 */

#pragma once

#include "xo/expression/Expression.hpp"
#include "xo/tokenizer/token.hpp"
#include <stack>
#include <stdexcept>

namespace xo {
    namespace scm {
        // ----- exprir -----

        enum class exprirtype {
            invalid = -1,

            empty,
            symbol,
            expression,
            typedescr,

            n_exprirtype
        };

        extern const char *
        exprirtype_descr(exprirtype x);

        inline std::ostream &
        operator<< (std::ostream & os,
                    exprirtype x)
        {
            os << exprirtype_descr(x);
            return os;
        }

        /** intermediate representation for some part of an expression
         *
         *  Examples:
         *  1. a variable name (but without type information)
         **/
        class exprir {
        public:
            using Expression = xo::ast::Expression;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            exprir() = default;
            exprir(exprirtype xir_type,
                   const std::string & x)
                : xir_type_{xir_type}, symbol_name_{x} {}
            exprir(exprirtype xir_type,
                   rp<Expression> expr)
                : xir_type_{xir_type}, expr_{std::move(expr)} {}
            exprir(exprirtype xir_type,
                   TypeDescr td)
                : xir_type_{xir_type}, td_{td} {}

            exprirtype xir_type() const { return xir_type_; }
            const std::string & symbol_name() const { return symbol_name_; }
            const rp<Expression> & expr() const { return expr_; }
            TypeDescr td() const { return td_; }

            void print(std::ostream & os) const;

        private:
            /** IR type code **/
            exprirtype xir_type_ = exprirtype::invalid;
            /** xir_type=symbol: a symbol (type or variable) name **/
            std::string symbol_name_;
            /** xir_type=expression: a completed expression **/
            rp<Expression> expr_;
            /** xir_type=typedescr: object identifying/describing a datatype **/
            TypeDescr td_ = nullptr;
        };

        inline std::ostream &
        operator<< (std::ostream & os, const exprir & x) {
            x.print(os);
            return os;
        }

        enum class exprstatetype {
            invalid = -1,

            /** toplevel of some translation unit **/
            expect_toplevel_expression_sequence,

            def_0,
            def_1,
            def_2,
            def_3,
            def_4,

            expect_rhs_expression,
            expect_symbol,
            expect_type,

            n_exprstatetype
        };

        extern const char *
        exprstatetype_descr(exprstatetype x);

        inline std::ostream &
        operator<< (std::ostream & os, exprstatetype x) {
            os << exprstatetype_descr(x);
            return os;
        }

        enum class expractiontype {
            invalid = -1,

            push1,
            push2,
            keep,
            emit,
            pop,

            n_expractiontype
        };

        extern const char *
        expractiontype_descr(expractiontype x);

        inline std::ostream &
        operator<< (std::ostream & os, expractiontype x) {
            os << expractiontype_descr(x);
            return os;
        }

        /** an action associated with parser response to an incoming lexical
         **/
        class expraction {
        public:
            expraction() = default;
            expraction(expractiontype action_type,
                       const exprir & expr_ir,
                       exprstatetype push_exs1,
                       exprstatetype push_exs2)
                : action_type_{action_type}, expr_ir_{expr_ir},
                  push_exs1_{push_exs1}, push_exs2_{push_exs2}
                {}

            static expraction keep();
            static expraction emit(const exprir & ir);
            static expraction push2(exprstatetype s1, exprstatetype s2);
            static expraction pop(const exprir & ir);

            expractiontype action_type() const { return action_type_; }
            const exprir & expr_ir() const { return expr_ir_; }
            exprstatetype push_exs1() const { return push_exs1_; }
            exprstatetype push_exs2() const { return push_exs2_; }

            void print(std::ostream & os) const;

        private:
            /**
             * push1: push new exprstate built from push_exs1_
             * push2: push new exprstate built from push_exs1_,
             *        followed by push_exs2_
             * keep: keep current exprstate (which will have updated inplace)
             * pop:  drop exprstate,  report exprir to parent
             **/
            expractiontype action_type_ = expractiontype::invalid;
            /**
             * intermediate representation (pass to enclosing stack state)
             **/
            exprir expr_ir_;
            /** with action_type push1 or push2,
             *  parser will push exprstate with this type
             **/
            exprstatetype push_exs1_ = exprstatetype::invalid;
            /** with action_type push2,
             *  parser will push exprstate with this type
             *  (after pushing exprstate built from push_exs1_)
             **/
            exprstatetype push_exs2_ = exprstatetype::invalid;
        };

        inline std::ostream &
        operator<< (std::ostream & os,
                    const expraction & x)
        {
            x.print(os);
            return os;
        }

        /** state associated with a partially-parsed expression.
         **/
        class exprstate {
        public:
            using exprtype = xo::ast::exprtype;
            using token_type = token<char>;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            exprstate() = default;
            exprstate(exprstatetype exs_type) : exs_type_{exs_type} {}

            static exprstate expect_toplevel_expression_sequence() {
                return exprstate(exprstatetype::expect_toplevel_expression_sequence);
            }
            static exprstate def_0() {
                return exprstate(exprstatetype::def_0);
            }
            static exprstate expect_symbol() {
                return exprstate(exprstatetype::expect_symbol);
            }

            exprstatetype exs_type() const { return exs_type_; }

            /** true iff this parsing state admits a 'def' keyword
             *  as next token
             **/
            bool admits_definition() const;
            /** true iff this parsing state admits a symbol as next token **/
            bool admits_symbol() const;
            /** true iff this parsing state admits a colon as next token **/
            bool admits_colon() const;
            /** true iff this parsing state admits a singleassign '=' as next token **/
            bool admits_singleassign() const;
            /** true iff this parsing state admits a 64-bit floating point literal token **/
            bool admits_f64() const;

            /** update exprstate in response to incoming token @p tk,
             *  forward instructions to parent parser
             **/
            expraction on_input(const token_type & tk);
            /** update exprstate in response to IR (intermediate representation)
             *  from nested parsing task
             **/
            expraction on_exprir(const exprir & ir);

            /** print human-readable representation on @p os **/
            void print(std::ostream & os) const;

        private:
            expraction on_def();
            expraction on_symbol(const token_type & tk);
            expraction on_colon();
            expraction on_singleassign();
            expraction on_f64(const token_type & tk);

        private:
            /**
             *   def foo : f64 = 1
             *  ^   ^   ^ ^   ^ ^ ^
             *  |   |   | |   | | (done)
             *  |   |   | |   | def_4:expect_rhs_expression
             *  |   |   | |   def_3
             *  |   |   | def_2:expect_type
             *  |   |   def_1
             *  |   def_0:expect_symbol
             *  expect_toplevel_expression_sequence
             *
             *   def_0:expect_symbol: got 'def' keyword, symbol to follow
             *   def_1: got symbol name
             *   def_2:expect_symbol got (optional) colon, type name to follow
             *   def_3: got symbol type
             *   def_4:expect_rhs_expression got (optional) equal sign, value to follow
             *   (done): definition complete,  pop exprstate from stack
             *
             **/
            exprstatetype exs_type_;

            /** e.g. foo in
             *    def foo : f64 = 1
             **/
            std::string def_lhs_symbol_;
            /** e.g. f64 in
             *    def foo : f64 = 1
             **/
            TypeDescr def_lhs_td_ = nullptr;
        }; /*exprstate*/

        inline std::ostream &
        operator<< (std::ostream & os, const exprstate & x) {
            x.print(os);
            return os;
        }

        /** @class exprstatestack
         *  @brief A stack of exprstate objects
         **/
        class exprstatestack {
        public:
            exprstatestack() {}

            bool empty() const { return stack_.empty(); }
            std::size_t size() const { return stack_.size(); }

            exprstate & top_exprstate();
            void push_exprstate(const exprstate & exs);
            void pop_exprstate();

            /** relative to top-of-stack.
             *  0 -> top (last in),  z-1 -> bottom (first in)
             **/
            exprstate & operator[](std::size_t i) {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1];
            }

            const exprstate & operator[](std::size_t i) const {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1];
            }

            void print (std::ostream & os) const;

        private:
            std::vector<exprstate> stack_;
        };

        /** schematica parser
         *
         *  Examples:
         *
         *    decltype point
         *
         *    // forward declarations
         *    decl pi : f64
         *    decl fib(n : i32) -> i32
         *
         *    def pi = 3.14159265  // constant. = is single assignment
         *
         *    def fib(n : i32) -> i32 {
         *      // nested defs ok
         *      def aux(n : i32, s1 : i32, s2 : i32) -> i32 {
         *        // or:
         *        //   (n == 0) ? s1 : aux(n - 1, s1 + s2, s1)
         *        //
         *        if (n == 0) {
         *          s1
         *        } else {
         *          aux(n - 1, s1 + s2, s1)
         *        }
         *
         *        // or:
         *        //  if (n == 0) ? s1 : aux(n - 1, s1 + s2, s1)
         *      }
         *
         *      aux(n=n, s1=1, s2=0)
         *    }
         *
         *    def anotherfib = lambda(n : i32) { fib(n) }
         *
         *    def any : object
         *    def l : list<object> = '()
         *
         *    deftype point :: {x : f64, y : f64}
         *    deftype polar :: {arg : f64, mag : f64}
         *
         *    def polar2rect(pt : polar) -> point {
         *      point(x = pt.mag * cos(arg),
         *            y = pt.mag * sin(arg))
         *    }
         *
         * Grammar:
         *   toplevel-program = expression*
         *   type-decl        = decltype $typename [<$tp1 .. $tpn>]
         *   expression       = define-expr
         *                       | literal-expr
         *                       | variable-expr
         *                       | apply-expr
         *                       | if-expr
         *                       | lambda-expr
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
         *                       | (test-expr) ? then-expr : else-expr
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
                    return xs_stack_[i].exs_type();
                }

                /* out of bounds */
                return exprstatetype::invalid;
            }

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
