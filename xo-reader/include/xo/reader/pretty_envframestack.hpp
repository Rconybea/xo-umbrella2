/* file pretty_envframestack.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "envframestack.hpp"
#include "xo/indentlog/print/pretty.hpp"

namespace xo {
    namespace print {
        template <>
        struct ppdetail<xo::scm::envframestack*> {
            static inline bool print_pretty(const ppindentinfo & ppii, const xo::scm::envframestack * p) {
                return p->pretty_print(ppii);
            }
        };

    } /*namespace print*/
} /*namespace xo*/
