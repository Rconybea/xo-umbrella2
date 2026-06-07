/** @file DArrayType.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Type.hpp"
#include "Metatype.hpp"
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>

namespace xo {
    namespace scm {
        /** @brief A parameterized array type: array(T)
         *
         *  Represents a fixed-size homogeneous collection.
         **/
        class DArrayType {
        public:
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;
            using AAllocator = xo::mm::AAllocator;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            /** @defgroup xo-scm-arraytype-ctors **/
            ///@{

            explicit DArrayType(obj<AType> elt);

            /** create instance using memory from @p mm with element type @p elt_type **/
            static DArrayType * _make(obj<AAllocator> mm, obj<AType> elt_type);

            ///@}
            /** @defgroup xo-scm-arraytype-type-facet **/
            ///@{
            Metatype metatype() const noexcept { return Metatype::t_array(); }
            TypeDescr repr_td() const noexcept;
            bool is_equal_to(const obj<AType> & y) const noexcept;
            bool is_subtype_of(const obj<AType> & y) const noexcept;
            ///@}
            /** @defgroup xo-scm-arraytype-gcobject-facet **/
            ///@{
            DArrayType * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;
            ///@}

        private:
            /** @defgroup xo-scm-arraytype-member-vars **/
            ///@{

            /** all array elements satisfy @ref elt_type_ **/
            obj<AType> elt_type_;

            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DArrayType.hpp */
