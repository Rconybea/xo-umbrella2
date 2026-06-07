/** @file DTypeVarRef.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Type.hpp"
#include "Metatype.hpp"
#include <xo/alloc2/Allocator.hpp>
#include <xo/stringtable2/UniqueString.hpp>

namespace xo {
    namespace scm {
        /** @brief An atomic schematika type
         *
         *  Types that are not parameterized by types or values.
         *  For example
         *    unit, bool, i64, f64
         *  are atomic in this sense.
         **/
        class DTypeVarRef {
        public:
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;
            using AAllocator = xo::mm::AAllocator;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            /** @defgroup xo-scm-typevarref-ctors constructors **/
            ///@{

            explicit DTypeVarRef(const DUniqueString * name) : name_{name} {}

            /** create instance using memory from @p mm with metatype @p mtype **/
            static DTypeVarRef * _make(obj<AAllocator> mm, const DUniqueString * name);
            /** create instance **/
            static obj<AType,DTypeVarRef> make(obj<AAllocator> mm, const DUniqueString * name);

            ///@}
            /** @defgroup xo-scm-typevarref-general-methods general methods **/
            ///@{

            const DUniqueString * name() const noexcept { return name_; }

            ///@}
            /** @defgroup xo-scm-atomictype-type-facet **/
            ///@{
            Metatype metatype() const noexcept;
            TypeDescr repr_td() const noexcept;
            bool is_equal_to(const obj<AType> & y) const noexcept;
            bool is_subtype_of(const obj<AType> & y) const noexcept;
            ///@}
            /** @defgroup xo-scm-atomictype-gcobject-facet **/
            ///@{
            DTypeVarRef * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;
            ///@}

        private:
            /** unique type-variable name **/
            const DUniqueString * name_ = nullptr;

            /** resolved type (if/when established) **/
            obj<AType> type_;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DTypeVarRef.hpp */
