/** @file ParserResult.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/expression2/Expression.hpp>
#include <xo/stringtable2/DString.hpp>
#include <string_view>

namespace xo {
    namespace scm {
        enum class parser_result_type {
            /** no result yet (no input or incomplete expression) **/
            none,
            /** emit expression **/
            expression,
            /** emit parsing error **/
            error,
            N
        };

        /** @return string representation for enum @p x **/
        const char * parser_result_type_descr(parser_result_type x);

        inline std::ostream & operator<<(std::ostream & os, parser_result_type x) {
            os << parser_result_type_descr(x);
            return os;
        }

        class ParserResult {
        public:
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;

        public:
            ParserResult() = default;
            ParserResult(parser_result_type type,
                         obj<AExpression> expr,
                         std::string_view error_src_fn,
                         const DString * error_description);

            /** create ParserResult for parsing success;
             *  parsing yields expression @p expr
             **/
            static ParserResult expression(std::string_view ssm,
                                           obj<AExpression> expr);

            /** create ParserResult for a parsing error.
             *  Reporting detailed message @p errmsg
             *  from syntax state machine @p ssm
             **/
            static ParserResult error(std::string_view ssm,
                                      const DString * errmsg);

            parser_result_type result_type() const { return result_type_; }
            obj<AExpression> result_expr() const { return result_expr_; }
            const DString * error_description() const { return error_description_; }

            bool is_incomplete() const { return result_type_ == parser_result_type::none; }
            bool is_expression() const { return result_type_ == parser_result_type::expression; }
            bool      is_error() const { return result_type_ == parser_result_type::error; }

            /** ordinary not-pretty printer **/
            void print(std::ostream & os) const;

            /** structured pretty-printing: render into @p sink **/
            void pretty(xo::pp::PpSink & sink) const;

            /** gc support: forward gc-eligible children **/
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;

        public:
            /** none|expression|error_description
             *
             *  @text
             *   result_type | error_src_function | error_description
             *  -------------+--------------------+-------------------
             *          none |            nullptr |             empty
             *    expression |            nullptr |             empty
             *         error |           non-null |         non-empty
             *  @endtext
             **/
            parser_result_type result_type_ = parser_result_type::none;
            /** non-null iff @ref result_type_ is expression **/
            obj<AExpression> result_expr_;
            /** non-null iff @ref result_type_ is error.
             *  In which case gives parsing function detecting this error
             **/
            std::string_view error_src_fn_;
            /** non-null iff @ref result_type_ is error
             *  Human-targeted error description.
             **/
            const DString * error_description_ = nullptr;
        };

        inline std::ostream & operator<<(std::ostream & os, const ParserResult & x) {
            x.print(os);
            return os;
        }

    } /*namespace scm*/

    namespace pp {
        /** ParserResult is NOT a facet type, so ppsink cannot reach it
         *  through APrintable the way the D-types are reached.  Without this
         *  specialization it falls through to ppsink's leaf FALLBACK to
         *  operator<<, i.e. to ParserResult::print(std::ostream&) -- which
         *  renders a DIFFERENT struct (always :expr and :src_fn, quoted
         *  :error, and never wrapping).  Adding it is what makes .pretty()
         *  reachable at all; the phase-B stub it replaced was dead code.
         *
         *  Two more printers in this subsystem are in the same position:
         *  ParserStack* and DSchematikaParser*, both still on ppdetail only.
         **/
        template <>
        struct Prettifier<xo::scm::ParserResult> {
            static void print(PpSink & sink, const xo::scm::ParserResult & x) {
                x.pretty(sink);
            }
        };
    } /*namespace pp*/
} /*namespace xo*/

/* end ParserResult.hpp */
