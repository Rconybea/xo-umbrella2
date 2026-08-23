/** @file ParserResult.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "ParserResult.hpp"
#include <xo/stringtable2/String.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::pp::field;
    using xo::print::APrintable;
    using xo::facet::FacetRegistry;

    namespace scm {

        const char *
        parser_result_type_descr(parser_result_type x)
        {
            switch (x) {
            case parser_result_type::none: return "none";
            case parser_result_type::expression: return "expression";
            case parser_result_type::error: return "error";
            case parser_result_type::N: break;
            }

            return "parser_result_type?";
        }

        ParserResult::ParserResult(parser_result_type type,
                                   obj<AExpression> expr,
                                   std::string_view error_src_fn,
                                   const DString * error_description)
                : result_type_{type},
                  result_expr_{expr},
                  error_src_fn_{error_src_fn},
                  error_description_{error_description}
        {}

        ParserResult
        ParserResult::expression(std::string_view ssm_name,
                                 obj<AExpression> expr)
        {
            return ParserResult(parser_result_type::expression,
                                expr,
                                ssm_name,
                                nullptr);
        }

        ParserResult
        ParserResult::error(std::string_view ssm_name,
                            const DString * errmsg)
        {
            return ParserResult(parser_result_type::error,
                                obj<AExpression>(),
                                ssm_name,
                                errmsg);
        }

        void
        ParserResult::pretty(xo::pp::PpSink & sink) const
        {
            /* The switch STAYS.  Unlike DLambdaSsm's if/else, these three
             * arms have three different ARITIES -- :type / :type :expr /
             * :type :src_fn :error -- and collapsing them onto field()'s
             * present flag would mean calling variant<APrintable,AExpression>
             * on the none and error paths, where result_expr_ is null and
             * that call throws.
             */
            switch (result_type_) {
            case parser_result_type::none:
                sink.pretty_struct("ParserResult",
                                   field("type", result_type_));
                return;

            case parser_result_type::expression:
                {
                    auto expr = (FacetRegistry::instance()
                                 .variant<APrintable,AExpression>(result_expr_));

                    sink.pretty_struct("ParserResult",
                                       field("type", result_type_),
                                       field("expr", expr));
                }
                return;

            case parser_result_type::error:
                sink.pretty_struct("ParserResult",
                                   field("type", result_type_),
                                   field("src_fn", error_src_fn_),
                                   field("error", error_description_));
                return;

            case parser_result_type::N:
                assert(false);
                break;
            }
        }

        void
        ParserResult::visit_gco_children(VisitReason reason,
                                         obj<AGCObjectVisitor> gc) noexcept
        {
            // {result_type_, error_src_fn_}: pod, ignore

            gc.visit_poly_child(reason, &result_expr_);
            gc.visit_child(reason, &error_description_);
        }
    } /*namespace scm*/

    namespace pp {
        XO_PRETTIFIER_VIA_CONVERSION(xo::scm::parser_result_type, xo::scm::parser_result_type_descr)
        XO_PRETTIFIER_VIA_PRETTY_METHOD(xo::scm::ParserResult);
    }
} /*namespace xo*/

/* end ParserResult.cpp */
