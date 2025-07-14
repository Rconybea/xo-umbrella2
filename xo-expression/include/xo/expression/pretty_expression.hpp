/* @file pretty_expression.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "xo/indentlog/print/pretty.hpp"
#include "xo/indentlog/print/pretty_tag.hpp"
#include "xo/refcnt/pretty_refcnt.hpp"
#include "Expression.hpp"

namespace xo {
    namespace print {
        template<>
        struct ppdetail<xo::ast::GeneralizedExpression> {
            static bool print_upto(ppstate * pps, const xo::ast::GeneralizedExpression & x) {
                return x.pretty_print(pps, true);
            }

            static void print_pretty(ppstate * pps, const xo::ast::GeneralizedExpression & x) {
                x.pretty_print(pps, false);
            }
        };

        template <>
        struct ppdetail<xo::ast::Expression> {
            static bool print_upto(ppstate * pps, const xo::ast::Expression & x) {
                return x.pretty_print(pps, true);
            }

            static void print_pretty(ppstate * pps, const xo::ast::Expression & x) {
                x.pretty_print(pps, false);
            }
        };
    }
}
