/* file pretty_variable.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "Variable.hpp"
#include "pretty_expression.hpp"

namespace xo {
    namespace print {
        template <>
        struct ppdetail<xo::scm::Variable> {
            static bool print_pretty(const ppindentinfo & ppii, const xo::scm::Variable & x) {
                return x.pretty_print(ppii);
            }
        };

        template <>
        struct ppdetail<xo::scm::Variable *> {
            static bool print_pretty(const ppindentinfo & ppii, const xo::scm::Variable * x) {
                return x->pretty_print(ppii);
            }
        };
    }
}
