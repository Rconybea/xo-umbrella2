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

        std::uint32_t
        GeneralizedExpression::pretty_print(ppstate * pps, bool upto) const {
            // Slooooow fallback for subtypes that don't implement pretty printing support
            // Currently have support for:
            // - Lambda

            std::uint32_t saved = pps->pos();
            pps->write(display_string());
            if (upto && !pps->has_margin())
                return false;

            return upto ? pps->scan_no_newline(saved) : true;
        }
    } /*namespace ast*/
} /*namespace xo*/

/* end GeneralizedExpression.cpp */
