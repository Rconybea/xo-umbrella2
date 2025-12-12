/** @file RAllocator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "xo/facet/RRouter.hpp"

namespace xo {
    namespace mm {
        /** @class RAllocator **/
        template <typename Object>
        struct RAllocator : public Object {
            using ObjectType = Object;

            RAllocator() {}
            RAllocator(Object::DataPtr data) : Object{std::move(data)} {}

            int32_t _typeseq() const { return Object::iface()->_typeseq(); }

            static bool _valid;
        };

        template <typename Object>
        bool
        RAllocator<Object>::_valid = facet::valid_object_router<Object>();
    }

    namespace facet {
        template <typename Object>
        struct RoutingFor<xo::mm::AAllocator, Object> {
            using RoutingType = xo::mm::RAllocator<Object>;
        };
    }
}

/* end RAllocator.hpp */
