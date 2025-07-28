/** @file GeneralizedExpression.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include "xo/expression/typeinf/type_ref.hpp"
//#include "xo/reflect/TypeDescr.hpp"
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
            using type_ref     = xo::scm::type_ref;
            using prefix_type  = xo::scm::prefix_type;
            using TypeDescr    = xo::reflect::TypeDescr;
            using ppstate      = xo::print::ppstate;
            using ppindentinfo = xo::print::ppindentinfo;

        public:
            /** if @p valuetype is null, generate unique type variable
             *  using prefix derived from @p extype.
             **/
            GeneralizedExpression(exprtype extype, TypeDescr valuetype);
            /** if @p valuetype is null, generate unique type variable
             *  name, beginning with @p prefix
             **/
            GeneralizedExpression(exprtype extype, prefix_type prefix, TypeDescr valuetype);

            exprtype extype() const { return extype_; }
            const type_ref & valuetype_ref() const { return valuetype_ref_; }
            TypeDescr valuetype() const { return valuetype_ref_.td(); }

            /** write human-readable representation to stream @p os **/
            virtual void display(std::ostream & os) const = 0;
            /** human-readable string representation **/
            virtual std::string display_string() const;
            /** pretty printing support.  See [xo-indentlog/xo/indentlog/pretty.hpp] **/
            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const = 0;

            /** useful when scaffolding expressions in a parser **/
            void assign_valuetype(TypeDescr x) { valuetype_ref_.resolve_to(x); }

        private:
            /** expression type (constant | apply | ..) for this expression **/
            exprtype extype_ = exprtype::invalid;
            /** type information (when available) for values produced by this
             *  expression.
             **/
            type_ref valuetype_ref_;
        };

        inline std::ostream &
        operator<<(std::ostream & os, const GeneralizedExpression & x) {
            x.display(os);
            return os;
        }
    } /*namespace ast*/
} /*namespace xo*/


/* end GeneralizedExpression.hpp */
