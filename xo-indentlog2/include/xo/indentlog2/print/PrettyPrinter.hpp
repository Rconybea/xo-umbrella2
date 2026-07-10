/** @file PrettyPrinter.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpState.hpp"

namespace xo {
    namespace print {
        /**
         *  Use:
         *  @code
         *    PpConfig ppc;
         *    PrettyPrinter pp(ppc);
         *  @endcode
         **/
        class PrettyPrinter {
        public:
            PrettyPrinter(const PpConfig & cfg);

        private:
            /** Pretty printer state (+ driver) **/
            PpState pps_;
            /** Buffer for pretty-printed output **/
            LogBuffer logbuf_;
        };
    } /*namespace print*/
} /*namespace xo*/

/* end PrettyPrinter.hpp */
