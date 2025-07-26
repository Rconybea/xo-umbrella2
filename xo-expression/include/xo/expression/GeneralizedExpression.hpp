/** @file GeneralizedExpression.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include "exprtype.hpp"

namespace xo {
    namespace ast {
        /** @class GeneralizedExpression
         *  @brief abstract syntax tree (non-executable) for schematica
         *
         *  'Generalized' because it includes both kernel and macro expressions.
         *  Every macro expression automatically translates to an equivalent kernel expression.
         *  Kernel expressions are directly executable.
         **/
        class GeneralizedExpression : public ref::Refcount {
        public:
            using TypeDescr    = xo::reflect::TypeDescr;
            using ppstate      = xo::print::ppstate;
            using ppindentinfo = xo::print::ppindentinfo;

        public:
            GeneralizedExpression(exprtype extype, TypeDescr valuetype)
                : extype_{extype}, valuetype_{valuetype}{}

            exprtype extype() const { return extype_; }
            TypeDescr valuetype() const { return valuetype_; }

            /** write human-readable representation to stream @p os **/
            virtual void display(std::ostream & os) const = 0;
            /** human-readable string representation **/
            virtual std::string display_string() const;
            /** pretty printing support.  See [xo-indentlog/xo/indentlog/pretty.hpp] **/
            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const = 0;

            /** useful when scaffolding expressions in a parser **/
            void assign_valuetype(TypeDescr x) { valuetype_ = x; }

        private:
            /** expression type (constant | apply | ..) for this expression **/
            exprtype extype_ = exprtype::invalid;
            /** type information (when available) for values produced by this
             *  expression.
             **/
            TypeDescr valuetype_ = nullptr;
        };

        inline std::ostream &
        operator<<(std::ostream & os, const GeneralizedExpression & x) {
            x.display(os);
            return os;
        }
    } /*namespace ast*/
} /*namespace xo*/


/* end GeneralizedExpression.hpp */
