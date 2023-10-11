/* @file EventSource.hpp */

#pragma once

#include "ReactorSource.hpp"
#include "xo/callback/CallbackSet.hpp"

namespace xo {
    namespace reactor {
        template</*typename Event,*/
            typename Callback
            /*void (Callback::*member_fn)(Event const &)*/>
        class EventSource : public ReactorSource {
        public:
            using CallbackId = fn::CallbackId;

        public:
            virtual CallbackId add_callback(ref::rp<Callback> const & cb) = 0;
            virtual void remove_callback(CallbackId id) = 0;
        }; /*EventSource*/

    } /*namespace reactor*/
} /*namespace xo*/

/* end EventSource.hpp */
