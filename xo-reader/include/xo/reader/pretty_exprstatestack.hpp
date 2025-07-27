/* file pretty_exprstatestack.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "exprstatestack.hpp"
#include "xo/indentlog/print/pretty.hpp"

namespace xo {
    namespace print {
        template <>
        struct ppdetail<xo::scm::exprstatestack*> {
            static inline bool print_pretty(const ppindentinfo & ppii, const xo::scm::exprstatestack * p) {
                return p->pretty_print(ppii);
            }
        };

        template <>
        struct ppdetail<const xo::scm::exprstatestack*> {
            static inline bool print_pretty(const ppindentinfo & ppii, const xo::scm::exprstatestack * p) {
                return p->pretty_print(ppii);
            }
        };

        template <>
        struct ppdetail<xo::scm::exprstate*> {
            static inline bool print_pretty(const ppindentinfo & ppii, const xo::scm::exprstate * p) {
                return p->pretty_print(ppii);
            }
        };
    } /*namespace print*/
} /*namespace xo*/
