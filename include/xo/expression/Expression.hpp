/** @file Expression.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include "exprtype.hpp"

namespace xo {
    namespace ast {
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
         **/
        class Expression : public ref::Refcount {
        public:
            explicit Expression(exprtype extype) : extype_{extype} {}

            exprtype extype() const { return extype_; }

            /** write human-readable representation to stream **/
            virtual void display(std::ostream & os) const = 0;
            /** human-readable string representation **/
            virtual std::string display_string() const;

        private:
            /** expression type (constant | apply | ..) for this expression **/
            exprtype extype_ = exprtype::invalid;
        }; /*Expression*/

        inline std::ostream &
        operator<<(std::ostream & os, const Expression & x) {
            x.display(os);
            return os;
        }
    } /*namespace ast*/
} /*namespace xo*/

/** end Expression.hpp **/
