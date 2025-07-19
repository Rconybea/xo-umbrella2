/* file pretty_variable.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "pretty_expression.hpp"
#include "Variable.hpp"

namespace xo {
    namespace print {
        template <>
        struct ppdetail<xo::ast::Variable> {
            static bool print_pretty(const ppindentinfo & ppii, const xo::ast::Expression & x) {
                return x.pretty_print(ppii);
            }
        };
    }
}
