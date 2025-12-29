/** @file RCollector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "ACollector.hpp"
#include <xo/facet/RRouter.hpp>

namespace xo {
    namespace mm {
        /** @class RCollector **/
        template <typename Object>
        struct RCollector : public Object {
        private:
            using O = Object;
        public:
            using ObjectType = Object;
            using DataPtr = Object::DataPtr;
            using size_type = std::size_t;
            //using value_type = std::byte *;

            RCollector() = default;
            RCollector(DataPtr data) : Object{std::move(data)} {}

            int32_t _typeseq() const noexcept { return O::iface()->_typeseq(); }
            size_type allocated(generation g, role r) const noexcept { return O::iface()->allocated(O::data(), g, r); }
            size_type reserved(generation g, role r) const noexcept { return O::iface()->reserved(O::data(), g, r); }
            size_type committed(generation g, role r) const noexcept { return O::iface()->committed(O::data(), g, r); }

            void install_type(int32_t tseq, IGCObject_Any & iface) { return O::iface()->install_type(O::data(), tseq, iface); }
            void add_gc_root(int32_t tseq, Opaque * root) { O::iface()->add_gc_root(O::data(), tseq, root); }

            void forward_inplace(AGCObject * lhs_iface, void ** lhs_data) { O::iface()->forward_inplace(O::data(), lhs_iface, lhs_data); }

            static bool _valid;
        };

        template <typename Object>
        bool
        RCollector<Object>::_valid = facet::valid_object_router<Object>();
    } /*namespace mm*/

    namespace facet {
        template <typename Object>
        struct RoutingFor<xo::mm::ACollector, Object> {
            using RoutingType = xo::mm::RCollector<Object>;
        };
    }
} /*namespace xo*/

/* end RCollector.hpp */
