/* @file GeneralizedExpression.cpp */

#include "GeneralizedExpression.hpp"
#include "pretty_expression.hpp"
#include <cstdint>

namespace xo {
    namespace ast {
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
