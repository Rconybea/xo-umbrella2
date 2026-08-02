/** @file ppdetail_Printable.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "xo/printable2/Printable.hpp"
#include <xo/indentlog/print/pretty.hpp>

namespace xo {
    namespace print {
        template <typename DRepr>
        struct ppdetail<xo::facet::obj<APrintable, DRepr>> {
            static bool print_pretty(const ppindentinfo & ppii,
                                     const xo::facet::obj<APrintable, DRepr> & x) {
                return x.pretty(ppii);
            }
        };
    }
} /*namespace xo*/

/* end ppdetail_Printable.hpp */
