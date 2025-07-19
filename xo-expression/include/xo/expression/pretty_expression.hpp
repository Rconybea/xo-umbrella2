/* @file pretty_expression.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "xo/indentlog/print/pretty.hpp"
#include "xo/refcnt/pretty_refcnt.hpp"
#include "Expression.hpp"

namespace xo {
    namespace print {
        template<>
        struct ppdetail<xo::ast::GeneralizedExpression> {
            static bool print_pretty(const ppindentinfo & ppii,
                                     const xo::ast::GeneralizedExpression & x) {
                return x.pretty_print(ppii);
            }
        };

        template <>
        struct ppdetail<xo::ast::Expression> {
            static bool print_pretty(const ppindentinfo & ppii,
                                     const xo::ast::Expression & x) {
                return x.pretty_print(ppii);
            }
        };
    }
}
