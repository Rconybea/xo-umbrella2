/** @file DAtomicType.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Type.hpp"
#include "Metatype.hpp"
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/alloc2/Allocator.hpp>

namespace xo {
    namespace scm {
        /** @brief An atomic schematika type
         *
         *  Types that are not parameterized by types or values.
         *  For example
         *    unit, bool, i64, f64
         *  are atomic in this sense.
         **/
        class DAtomicType {
        public:
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;
            using AAllocator = xo::mm::AAllocator;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            explicit DAtomicType(Metatype m) : metatype_{m} {}

            /** create instance using memory from @p mm with metatype @p mtype **/
            static DAtomicType * _make(obj<AAllocator> mm, Metatype mtype);
            /** create instance **/
            static obj<AType,DAtomicType> make(obj<AAllocator> mm, Metatype mtype);

            /** @defgroup xo-scm-atomictype-type-facet **/
            ///@{
            Metatype metatype() const noexcept { return metatype_; }
            TypeDescr repr_td() const noexcept;
            bool is_equal_to(const obj<AType> & y) const noexcept;
            bool is_subtype_of(const obj<AType> & y) const noexcept;
            ///@}
            /** @defgroup xo-scm-atomictype-gcobject-facet **/
            ///@{
            DAtomicType * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;
            ///@}

        private:
            Metatype metatype_;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DAtomicType.hpp */
