/* @file DirectSourcePtr.hpp */

#pragma once

#include "EventTimeFn.hpp"
#include "LastReducer.hpp"
#include "SecondarySource.hpp"

namespace xo {
    namespace reactor {
        template<typename Event>
        using DirectSource = SecondarySource<Event,
                                             LastReducer<Event,
                                                         StructEventTimeFn<Event>>>;

        /* use when Event is rp<T> for some T */
        template<typename Event>
        using DirectSourcePtr = SecondarySource<Event,
                                                LastReducer<Event,
                                                            PtrEventTimeFn<Event>>>;

    } /*namespace reactor*/
} /*namespace xo*/

/* end DirectSourcePtr.hpp */
