/* @file GeneralizedExpression.cpp */

#include "GeneralizedExpression.hpp"
#include "pretty_expression.hpp"
#include <cstdint>

namespace xo {
    namespace ast {
        namespace {
            using xo::scm::prefix_type;

            prefix_type exprtype2prefix(exprtype x)
            {
                switch (x) {
                case exprtype::invalid:   assert(false); break;
                case exprtype::constant:  return prefix_type::from_chars("k");
                case exprtype::primitive: return prefix_type::from_chars("pm");
                case exprtype::define:    return prefix_type::from_chars("def");
                case exprtype::assign:    return prefix_type::from_chars("=");
                case exprtype::apply:     return prefix_type::from_chars("@");
                case exprtype::lambda:    return prefix_type::from_chars("lm");
                case exprtype::variable:  return prefix_type::from_chars("var");
                case exprtype::ifexpr:    return prefix_type::from_chars("if");
                case exprtype::sequence:  return prefix_type::from_chars("seq");
                case exprtype::convert:   return prefix_type::from_chars("cvt");
                case exprtype::n_expr:    assert(false); break;
                }

                return prefix_type::from_chars("?expr");
            }
        }

        GeneralizedExpression::GeneralizedExpression(exprtype extype,
                                                     TypeDescr valuetype)
            : extype_{extype},
              valuetype_ref_{type_ref::dwim(exprtype2prefix(extype), valuetype)}
        {}

        GeneralizedExpression::GeneralizedExpression(exprtype extype,
                                                     prefix_type prefix,
                                                     TypeDescr valuetype)
            : extype_{extype},
              valuetype_ref_{type_ref::dwim(prefix, valuetype)}
        {}

        std::string
        GeneralizedExpression::display_string() const {
            return tostr(*this);
        }

#ifdef SUPERSEDED // currently all derived expression types support pretty printing
        std::uint32_t
        GeneralizedExpression::pretty_print(const ppindentinfo & ppii) const {
            // Slooooow fallback for subtypes that don't implement pretty printing support
            // Currently have support for:
            // - Variable
            // - Lambda
            // - DefineExpr
            // - Sequence
            // - Apply
            // - Primitive
            // - IfExpr

            ppstate * pps = ppii.pps();
            std::uint32_t saved = pps->pos();
            pps->write(display_string());
            if (ppii.upto() && !pps->has_margin())
                return false;

            return ppii.upto() ? pps->scan_no_newline(saved) : true;
        }
#endif
    } /*namespace ast*/
} /*namespace xo*/

/* end GeneralizedExpression.cpp */
