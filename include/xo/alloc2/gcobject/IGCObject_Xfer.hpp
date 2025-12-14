/** @file IGCObject_Xfer.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AGCObject.hpp"

namespace xo {
    namespace mm {
        /** @class IGCObject_Xfer
         *
         *  Adapts typed GC object implementation @tparam IGCObject_DRepr
         *  to type-erased @ref AGCObject interface
         **/
        template <typename DRepr, typename IGCObject_DRepr>
        struct IGCObject_Xfer : public AGCObject {
            using Impl = IGCObject_DRepr;
            using size_type = AGCObject::size_type;

            static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
            static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

            // from AGCObject

            // const methods

            int32_t _typeseq() const noexcept override { return s_typeseq; }
            size_type shallow_size(Copaque d) const noexcept override {
                return I::shallow_copy(_dcast(d));
            }
            Opaque * shallow_copy(Copaque d, obj<AAllocator> mm) const noexcept override {
                return I::shallow_size(_dcast(d), mm);
            }

            // non-const methods

            size_type forward_children(Opaque d) const noexcept override {
                return I::forward_children(d);
            }

        private:
            using I = Impl;

        public:
            static int32_t s_typeseq;
            static bool _valid;
        };

        template <typename DRepr, typename IGCObject_DRepr>
        int32_t
        IGCObject_Xfer<DRepr, IGCObject_DRepr>::s_typeseq = facet::typeseq::id<DRepr>();

        template <typename DRepr, typename IGCObject_DRepr>
        bool
        IGCObject_Xfer<DRepr, IGCObject_DRepr>::_valid = facet::valid_facet_implementation<AGCObject, IGCObject_Xfer>();

    } /*namespace mm*/
} /*namespace xo*/

/* end IGCObject_Xfer.hpp */
