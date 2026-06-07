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
        struct ppdetail<xo::scm::GeneralizedExpression> {
            static bool print_pretty(const ppindentinfo & ppii,
                                     const xo::scm::GeneralizedExpression & x) {
                return x.pretty_print(ppii);
            }
        };

        template <>
        struct ppdetail<xo::scm::Expression> {
            static bool print_pretty(const ppindentinfo & ppii,
                                     const xo::scm::Expression & x) {
                return x.pretty_print(ppii);
            }
        };

    }
}
