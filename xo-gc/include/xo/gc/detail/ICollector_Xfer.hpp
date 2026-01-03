/** @file ICollector_Xfer.hpp
 *
 *  @author Roland Conybeare, 2025
 **/

#pragma once

#include "ACollector.hpp"
#include "AGCObject.hpp"

namespace xo {
    namespace mm {
        /** @class ICollector_Xfer
         *
         *  Adapts typed ACollector implementation @tparam ICollector_DRepr
         *  to type-erased @ref ACollector interface
         *
         *  See for example
         *  @ref ICollector_DX1Collector
         **/
        template <typename DRepr, typename ICollector_DRepr>
        struct ICollector_Xfer : public ACollector {
        public:
            using Impl = ICollector_DRepr;
            using size_type = ACollector::size_type;

            static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
            static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

            // from ACollector

            // const methods

            typeseq _typeseq() const noexcept override { return s_typeseq; }
            size_type allocated(Copaque d, generation g, role r) const noexcept override {
                return I::allocated(_dcast(d), g, r);
            }
            size_type reserved(Copaque d, generation g, role r) const noexcept override {
                return I::reserved(_dcast(d), g, r);
            }
            size_type committed(Copaque d, generation g, role r) const noexcept override {
                return I::committed(_dcast(d), g, r);
            }
            bool is_type_installed(Copaque d, typeseq tseq) const noexcept override {
                return I::is_type_installed(_dcast(d), tseq);
            }

            // non-const methods

            bool install_type(Opaque d, const AGCObject & iface) override {
                return I::install_type(_dcast(d), iface);
            }
            void add_gc_root(Opaque d, int32_t tseq, Opaque * root) override {
                I::add_gc_root(_dcast(d), tseq, root);
            }
            void forward_inplace(Opaque d, AGCObject * lhs_iface, void ** lhs_data) override {
                I::forward_inplace(_dcast(d), lhs_iface, lhs_data);
            }

        private:
            using I = Impl;

        public:
            static typeseq s_typeseq;
            static bool _valid;
        };

        template <typename DRepr, typename ICollector_DRepr>
        xo::facet::typeseq
        ICollector_Xfer<DRepr, ICollector_DRepr>::s_typeseq = facet::typeseq::id<DRepr>();

        template <typename DRepr, typename ICollector_DRepr>
        bool
        ICollector_Xfer<DRepr, ICollector_DRepr>::_valid = facet::valid_facet_implementation<ACollector, ICollector_Xfer>();

    } /*namespace mm*/
} /*namespace xo*/

/* end ICollector_Xfer.hpp */
