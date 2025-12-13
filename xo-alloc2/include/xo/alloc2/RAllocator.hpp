/** @file RAllocator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "xo/facet/RRouter.hpp"
#include <string>

namespace xo {
    namespace mm {
        /** @class RAllocator **/
        template <typename Object>
        struct RAllocator : public Object {
        private:
            using O = Object;
        public:
            using ObjectType = Object;
            using size_type = std::size_t;
            using value_type = std::byte *;

            RAllocator() {}
            RAllocator(Object::DataPtr data) : Object{std::move(data)} {}

            int32_t _typeseq() const { return O::iface()->_typeseq(); }
            std::string_view name() const { return O::iface()->name(O::data()); }
            size_type reserved() const { return O::iface()->reserved(O::data()); }
            size_type size() const { return O::iface()->size(O::data()); }
            size_type committed() const { return O::iface()->committed(O::data()); }
            size_type available() const { return O::iface()->available(O::data()); }
            size_type allocated() const { return O::iface()->allocated(O::data()); }
            bool contains(const void * p) const { return O::iface()->contains(O::data(), p); }
            AllocatorError last_error() const { return O::iface()->last_error(O::data()); }

            bool expand(size_type z) { return O::iface()->expand(O::data(), z); }
            value_type alloc(size_type z) { return O::iface()->alloc(O::data(), z); }
            value_type super_alloc(size_type z) { return O::iface()->super_alloc(O::data(), z); }
            value_type sub_alloc(size_type z,
                                 bool complete_flag) { return O::iface()->sub_alloc(O::data(),
                                                                                    z, complete_flag); }

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
