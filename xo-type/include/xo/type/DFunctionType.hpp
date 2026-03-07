/** @file DFunctionType.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Type.hpp"
#include "Metatype.hpp"
#include <xo/object2/Array.hpp>
#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/Allocator.hpp>

namespace xo {
    namespace scm {
        /** @brief A parameterized array type: array(T)
         *
         *  Represents a fixed-size homogeneous collection.
         **/
        class DFunctionType {
        public:
            using ACollector = xo::mm::ACollector;
            using AAllocator = xo::mm::AAllocator;
            using AGCObject = xo::mm::AGCObject;

        public:
            /** @defgroup xo-scm-arraytype-ctors **/
            ///@{

            /** create instance using memory from @p mm,
             *  for a function with return type @p ret_type and arguments @p args
             **/
            template <typename... Args>
                requires (std::same_as<Args, obj<AType>> && ...)
            explicit DFunctionType(obj<AAllocator> mm, obj<AType> ret_type, Args... args);

            /** create instance using memory from @p mm,
             *  for a function with return type @p ret_type and arguments @p args
             **/
            template <typename... Args>
                requires (std::same_as<Args, obj<AType>> && ...)
            static DFunctionType * _make(obj<AAllocator> mm,
                                         obj<AType> ret_type, Args... args);

            ///@}
            /** @defgroup xo-scm-arraytype-type-facet **/
            ///@{
            Metatype metatype() const noexcept { return Metatype::array(); }
            bool is_equal_to(obj<AType> y) const noexcept;
            bool is_subtype_of(const obj<AType> & y) const noexcept;
            ///@}
            /** @defgroup xo-scm-arraytype-gcobject-facet **/
            ///@{
            std::size_t shallow_size() const noexcept;
            DFunctionType * shallow_copy(obj<AAllocator> mm) const noexcept;
            std::size_t forward_children(obj<ACollector> gc) noexcept;
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
            requires (std::same_as<Args, obj<AType>> && ...)
        DFunctionType::DFunctionType(obj<AAllocator> mm, obj<AType> return_type, Args... args)
            : return_type_{return_type},
              arg_types_{DArray::array(mm, args...)}
        {}

        template <typename... Args>
            requires (std::same_as<Args, obj<AType>> && ...)
        DFunctionType *
        DFunctionType::_make(obj<AAllocator> mm, obj<AType> ret_type, Args... args)
        {
            void * mem = mm.alloc_for<DFunctionType>();

            return new (mem) DFunctionType(mm, ret_type, args...);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DFunctionType.hpp */
