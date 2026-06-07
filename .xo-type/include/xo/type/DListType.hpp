/** @file DListType.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Type.hpp"
#include "Metatype.hpp"
#include <xo/reflect/TypeDescr.hpp>
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
        class DListType {
        public:
            using TypeDescr = xo::reflect::TypeDescr;
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;
            using AAllocator = xo::mm::AAllocator;

        public:
            /** @defgroup xo-scm-listtype-ctors **/
            ///@{

            explicit DListType(obj<AType> elt);

            /** create instance using memory from @p mm with element type @p elt_type **/
            static DListType * _make(obj<AAllocator> mm,
                                     obj<AType> elt_type);
            /** create fop to new instance using memory from @p mm with element type @p elt_type **/
            static obj<AType,DListType> make(obj<AAllocator> mm,
                                             obj<AType> elt_type);

            ///@}
            /** @defgroup xo-scm-listtype-type-facet **/
            ///@{
            Metatype metatype() const noexcept { return Metatype::t_list(); }
            TypeDescr repr_td() const noexcept;
            bool is_equal_to(const obj<AType> & y) const noexcept;
            bool is_subtype_of(const obj<AType> & y) const noexcept;
            ///@}
            /** @defgroup xo-scm-listtype-gcobject-facet **/
            ///@{
            DListType * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;
            ///@}

        private:
            /** @defgroup xo-scm-listtype-member-vars **/
            ///@{

            /** all list elements satisfy @ref elt_type_ **/
            obj<AType> elt_type_;

            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DListType.hpp */
