/** @file RAllocator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AAllocator.hpp"
#include "AllocIterator.hpp"
#include <xo/facet/RRouter.hpp>
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
            using DataPtr = Object::DataPtr;
            using typeseq = xo::facet::typeseq;
            using size_type = std::size_t;
            using value_type = std::byte *;
            using range_type = AAllocator::range_type;

            RAllocator() {}
            RAllocator(Object::DataPtr data) : Object{std::move(data)} {}

            typeseq       _typeseq() const noexcept { return O::iface()->_typeseq(); }
            void             _drop() const noexcept { O::iface()->_drop(O::data()); }
            std::string_view  name() const noexcept { return O::iface()->name(O::data()); }
            size_type     reserved() const noexcept { return O::iface()->reserved(O::data()); }
            size_type         size() const noexcept  { return O::iface()->size(O::data()); }
            size_type    committed() const noexcept { return O::iface()->committed(O::data()); }
            size_type    available() const noexcept { return O::iface()->available(O::data()); }
            size_type    allocated() const noexcept { return O::iface()->allocated(O::data()); }
            bool          contains(const void * p) const noexcept { return O::iface()->contains(O::data(), p); }
            AllocError  last_error() const noexcept { return O::iface()->last_error(O::data()); }
            AllocInfo   alloc_info(value_type mem) const noexcept { return O::iface()->alloc_info(O::data(), mem); }
            range_type alloc_range(DArena & mm) const noexcept { return O::iface()->alloc_range(O::data(), mm); }

            value_type       alloc(typeseq t, size_type z) noexcept { return O::iface()->alloc(O::data(), t, z); }
            value_type super_alloc(typeseq t, size_type z) noexcept { return O::iface()->super_alloc(O::data(), t, z); }
            value_type   sub_alloc(size_type z,
                                   bool complete_flag) noexcept { return O::iface()->sub_alloc(O::data(),
                                                                                             z, complete_flag); }
            value_type  alloc_copy(value_type src) noexcept { return O::iface()->alloc_copy(O::data(), src); }
            bool            expand(size_type z) { return O::iface()->expand(O::data(), z); }

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
