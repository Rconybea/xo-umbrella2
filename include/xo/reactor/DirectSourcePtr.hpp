/* @file DirectSourcePtr.hpp */

#pragma once

#include "SecondarySource.hpp"
#include "LastReducer.hpp"
#include "EventTimeFn.hpp"

namespace xo {
    namespace reactor {
        template<typename Event>
        using DirectSource = SecondarySource<Event,
                                             LastReducer<Event,
                                                         StructEventTimeFn<Event>>>;

        /* use when Event is ref::rp<T> for some T */
        template<typename Event>
        using DirectSourcePtr = SecondarySource<Event,
                                                LastReducer<Event,
                                                            PtrEventTimeFn<Event>>>;

    } /*namespace reactor*/
} /*namespace xo*/

/* end DirectSourcePtr.hpp */
