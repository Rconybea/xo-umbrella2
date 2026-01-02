/** @file RGCObject.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AGCObject.hpp"
#include <xo/facet/RRouter.hpp>

namespace xo {
    namespace mm {
        /** @class RGCObject **/
        template <typename Object>
        struct RGCObject : public Object {
        private:
            using O = Object;
        public:
            using ObjectType = Object;
            using DataPtr = Object::DataPtr;
            using typeseq = xo::facet::typeseq;
            using size_type = std::size_t;

            RGCObject() = default;
            RGCObject(Object::DataPtr data) : Object{std::move(data)} {}

            typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
            size_type shallow_size() const noexcept { O::iface()->shallow_size(O::data()); }
            Opaque shallow_copy(obj<AAllocator> mm) const noexcept { O::iface()->shallow_copy(O::data(), mm); }
            size_type forward_children() noexcept { O::iface()->forward_children(O::data()); }

            static bool _valid;
        };

        template <typename Object>
        bool
        RGCObject<Object>::_valid = facet::valid_object_router<RGCObject>();
    } /*namespace mm*/

    namespace facet {
        template <typename Object>
        struct RoutingFor<xo::mm::AGCObject, Object> {
            using RoutingType = xo::mm::RGCObject<Object>;
        };
    }
} /*namespace xo*/

/* end RGCObject.hpp */
