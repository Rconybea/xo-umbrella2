/** @file DFunctionType.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Type.hpp"
#include "Metatype.hpp"
#include <xo/object2/Array.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/alloc2/Allocator.hpp>

namespace xo {
    namespace scm {
        /** @brief A parameterized array type: array(T)
         *
         *  Represents a fixed-size homogeneous collection.
         **/
        class DFunctionType {
        public:
            using AGCObject = xo::mm::AGCObject;
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;
            using AAllocator = xo::mm::AAllocator;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            /** @defgroup xo-scm-arraytype-ctors **/
            ///@{

            /** create instance using memory from @p mm,
             *  for a function with return type @p ret_type and arguments @p args
             **/
            template <typename... Args>
                requires (std::convertible_to<Args, obj<AType>> && ...)
            explicit DFunctionType(obj<AAllocator> mm, obj<AType> ret_type, Args... args);

            /** create instance using memory from @p mm,
             *  for a function with return type @p ret_type and arguments @p args
             **/
            template <typename... Args>
                requires (std::convertible_to<Args, obj<AType>> && ...)
            static DFunctionType * _make(obj<AAllocator> mm,
                                         obj<AType> ret_type, Args... args);

#ifdef NOT_USING
            /** create instance using memory from @p mm
             *  for function with return type @p ret_type and arguments @p args
             **/
            template <typename... Args>
            requires (std::same_as<Args, obj<AType>> && ...)
            static obj<AType,DFunctionType> make(obj<AAllocator> mm,
                                                 obj<AType> ret_type, Args... args);
#endif

            ///@}
            /** @defgroup xo-scm-arraytype-type-facet **/
            ///@{
            Metatype metatype() const noexcept { return Metatype::t_array(); }
            TypeDescr repr_td() const noexcept;
            bool is_equal_to(obj<AType> y) const noexcept;
            bool is_subtype_of(const obj<AType> & y) const noexcept;
            ///@}
            /** @defgroup xo-scm-arraytype-gcobject-facet **/
            ///@{
            DFunctionType * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;
            ///@}

        private:
            /** @defgroup xo-scm-arraytype-member-vars **/
            ///@{

            /** function return type **/
            obj<AType> return_type_;

            /** argument types **/
            DArray * arg_types_ = nullptr;

            ///@}
        };

        template <typename... Args>
            requires (std::convertible_to<Args, obj<AType>> && ...)
        DFunctionType::DFunctionType(obj<AAllocator> mm, obj<AType> return_type, Args... args)
            : return_type_{return_type},
              arg_types_{DArray::array(mm, args.template to_facet<AGCObject>()...)}
        {}

        template <typename... Args>
            requires (std::convertible_to<Args, obj<AType>> && ...)
        DFunctionType *
        DFunctionType::_make(obj<AAllocator> mm, obj<AType> ret_type, Args... args)
        {
            void * mem = mm.alloc_for<DFunctionType>();

            return new (mem) DFunctionType(mm, ret_type, args...);
        }

#ifdef NOT_USING
        template <typename... Args>
            requires (std::same_as<Args, obj<AType>> && ...)
        obj<AType,DFunctionType>
        DFunctionType::make(obj<AAllocator> mm, obj<AType> ret_type, Args... args)
        {
            void * mem = mm.alloc_for<DFunctionType>();

            return obj<AType,DFunctionType>(_make(mm, ret_type, args...));
        }
#endif

    } /*namespace scm*/
} /*namespace xo*/

/* end DFunctionType.hpp */
