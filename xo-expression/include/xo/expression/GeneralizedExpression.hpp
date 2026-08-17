/** @file GeneralizedExpression.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprtype.hpp"
#include "typeinf/type_ref.hpp"
#include <xo/refcnt/Refcounted.hpp>
#include <xo/ppsink/pretty.hpp>
#include <concepts>

namespace xo {
    namespace scm {
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
            using PpSink       = xo::pp::PpSink;

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
            /** pretty-print to  sink.  Single-pass: the sink decides where groups
             *  break, so an implementation just emits its structure.  See
             *  xo-ppsink/pretty_struct.hpp (pretty_struct / struct_open).
             **/
            virtual void pretty(xo::pp::PpSink & sink) const = 0;

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
    } /*namespace scm*/
} /*namespace xo*/

namespace xo::pp {
    /* the hierarchy dispatches dynamically through the virtual
     * GeneralizedExpression::pretty(); these just enter it.
     */
    template <typename T>
    requires std::derived_from<T, xo::scm::GeneralizedExpression>
    struct Prettifier<T> {
        static void print(PpSink & sink, const T & x) {
            x.pretty(sink);
        }
    };
} /*namespace xo::pp*/

/* end GeneralizedExpression.hpp */
