/** @file Expression.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/reflect/TypeDescr.hpp"
#include "xo/refcnt/Refcounted.hpp"
#include "exprtype.hpp"
#include <functional>
#include <set>

namespace xo {
    namespace ast {
        class Variable; /* see Variable.hpp */
        class Lambda;   /* see Lamnbda.hpp */
        class Environment; /* see Environment.hpp */

        /** @class Expression
         *  @brief abstract syntax tree for an EGAD program
         *
         *  (Expression Graph with Automagic Derivation)
         *
         *  Things you can do with an Expression:
         *  - evaluate it using an interpreter
         *  - execute it on a VM
         *  - compile using LLVM
         *    see xo-jit/
         *
         *  Expressions are immutable.  This means they can resused
         *  across jit interactions
         *
         *  Every expression evaluates to a value with a particular type
         **/
        class Expression : public ref::Refcount {
        public:
            using VisitFn = std::function
                <void (ref::brw<Expression>)>;
            using TransformFn = std::function
                <rp<Expression> (ref::brw<Expression>)>;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            explicit Expression(exprtype extype, TypeDescr valuetype)
                : extype_{extype}, valuetype_{valuetype}{}

            exprtype extype() const { return extype_; }
            TypeDescr valuetype() const { return valuetype_; }

            /** find free named variables in this expression.
             *  comprises the set of names that don't match formal parameters in
             *  enclosing lambdas.
             **/
            virtual std::set<std::string> get_free_variables() const = 0;

            /** visit each Expression node in this AST,
             *  and invoke @p fn for each.
             *  Returns the number of nodes visited.
             *  Preorder: call @p fn for a node before visiting children
             **/
            virtual std::size_t visit_preorder(VisitFn visitor_fn) = 0;

            /** visit each Expression node in this AST,
             *  including immediately-nested Lambda nodes;
             *  but do not recurse into the params/body of such nested Lambdas.
             *  Returns the number of nodes visited
             **/
            virtual std::size_t visit_layer(VisitFn visitor_fn) = 0;

            /** traverse ast @ref visit_preorder but do not visit Lambdas **/
            virtual rp<Expression> xform_layer(TransformFn visitor_fn) = 0;

            /** attach an environment to each lambda expression X in this subtree,
             *  that will:
             *  - resolve names matching X's arguments (formal parameters) to
             *    from @p X.argv
             *  - resolve free variables from @p parent
             **/
            virtual void attach_envs(ref::brw<Environment> parent) = 0;

            /** append to *p_set the set of free variables in this expression.
             *  returns the number of free variables introduced
             *
             *  @param env   stack of lexcically-enclosing lamnbda expressions,
             *               in nesting order, i.e. outermost first, innertmost last
             **/
            //virtual std::int32_t find_free_vars(std::vector<ref::brw<Lambda>> env) = 0;

            /** write human-readable representation to stream **/
            virtual void display(std::ostream & os) const = 0;
            /** human-readable string representation **/
            virtual std::string display_string() const;

        protected:
            /** useful when scaffolding expressions in a parser **/
            void assign_valuetype(TypeDescr x) { valuetype_ = x; }

        private:
            /** expression type (constant | apply | ..) for this expression **/
            exprtype extype_ = exprtype::invalid;
            /** type information (when available) for values produced by this
             *  expression.
             **/
            TypeDescr valuetype_ = nullptr;
        }; /*Expression*/

        inline std::ostream &
        operator<<(std::ostream & os, const Expression & x) {
            x.display(os);
            return os;
        }
    } /*namespace ast*/
} /*namespace xo*/

/** end Expression.hpp **/
