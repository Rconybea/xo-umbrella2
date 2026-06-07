/* @file UpxToConsole.hpp */

#pragma once

#include "UpxEvent.hpp"
#include "xo/reactor/Sink.hpp"

namespace xo {
    namespace process {
        /* trivial extension of SinkToConsole<UpxEvent>.
         * hoping to workaroudn a typeinfo problem by getting typeinfo for Sink1<UpxEvent>
         * to appear in the process/ library instead of the process_py/ library.
         *
         * See FAQ "dynamic_cast<Foo<T> *> fails unexpectedly for a template class"
         */
        class UpxToConsole : public xo::reactor::SinkToConsole<UpxEvent> {
        public:
            UpxToConsole();

            static rp<UpxToConsole> make();
        }; /*UpxToConsole*/
    } /*namespace process*/
} /*namespace xo*/

/* end UpxToConsole.hpp */
