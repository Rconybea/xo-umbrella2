/* @file pretty_localenv.hpp */

#pragma once

#include "xo/indentlog/print/pretty.hpp"
#include "xo/refcnt/pretty_refcnt.hpp"
#include "LocalEnv.hpp"

namespace xo {
    namespace print {
        template <>
        struct ppdetail<xo::ast::LocalEnv> {
            static bool print_pretty(const ppindentinfo & ppii, const xo::ast::LocalEnv & x) {
                return x.pretty_print(ppii);
            }
        };
    }
}
