/* file pretty_parserstatemachine.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "parserstatemachine.hpp"
#include "xo/indentlog/print/pretty.hpp"

namespace xo {
    namespace print {
        template<>
        struct ppdetail<xo::scm::parserstatemachine> {
            static bool print_pretty(const ppindentinfo & ppii, const xo::scm::parserstatemachine & x);
        };
    }
} /*namespace xo*/
