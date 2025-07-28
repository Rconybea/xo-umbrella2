/* @file pretty_localenv.hpp */

#pragma once

#include "xo/indentlog/print/pretty.hpp"
#include "xo/refcnt/pretty_refcnt.hpp"
#include "LocalEnv.hpp"

namespace xo {
    namespace print {
        template <>
        struct ppdetail<xo::scm::LocalEnv> {
            static bool print_pretty(const ppindentinfo & ppii, const xo::scm::LocalEnv & x) {
                return x.pretty_print(ppii);
            }
        };

        template <>
        struct ppdetail<xo::scm::LocalEnv*> {
            static bool print_pretty(const ppindentinfo & ppii, const xo::scm::LocalEnv* x) {
                if (x) {
                    return x->pretty_print(ppii);
                } else {
                    ppii.pps()->write("<nullptr ");
                    ppii.pps()->write(reflect::type_name<xo::scm::LocalEnv>());
                    ppii.pps()->write(">");
                    return ppii.pps()->has_margin();
                }
            }
        };
    }
}
