/** @file DArray.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/gc/GCObject.hpp>
#include <xo/gc/Collector.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/obj.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <cstdint>

namespace xo {
    namespace scm {
        /** @class DArray
         *  @brief Polymorphic array implementation with gc hooks
         *
         *  1D Array implementation for Schematika
         *  Like DString, this implementation has max capacity
         *  fixed at construction time, but not part of type.
         *  Can reallocate to change
         **/
        struct DArray {
        public:
            /** @defgroup darray-types type traits **/
            ///@{

            /** type for array size **/
            using size_type = std::uint32_t;
            /** xo allocator facet **/
            using AAllocator = xo::mm::AAllocator;
            /** garbage collector facet **/
            using ACollector = xo::mm::ACollector;
            /** gc-aware object facet **/
            using AGCObject = xo::mm::AGCObject;
            /** pretty-printer state for APrintable **/
            using ppindentinfo = xo::print::ppindentinfo;

            ///@}
            /** @defgroup darray-ctors constructors **/
            ///@{

            /** default ctor. zero capacity sentinel **/
            DArray() = default;

            /** not simply copyable because of flexible array.
             *  Need allocator. See @ref clone
             **/
            DArray(const DArray &) = delete;

            /** create empty array with space for @p cap elements
             *  using memory from allocator @p mm.
             *  Nullptr if space exhausted
             **/
            static DArray * empty(obj<AAllocator> mm,
                                  size_type cap);

            ///@}
            /** @defgroup darray-access acecss methods **/
            ///@{
            /** true iff array is empty **/
            bool is_empty() const noexcept { return size_ == 0; }
            /** only support finite arrays :-) **/
            bool is_finite() const noexcept { return true; }
            /** return element @p index of this array (0-based) **/
            obj<AGCObject> at(size_type index) const;
            ///@}
            /** @defgroup darray-iterators iterators **/
            ///@{

            ///@}
            /** @defgroup darray-assign assignment **/
            ///@{
            /** append @p elt at the end of array.
             *  true on success, false otherwise
             **/
            bool push_back(obj<AGCObject> elt) noexcept;
            ///@}
            /** @defgroup darray-general general methods **/
            ///@{

            ///@}
            /** @defgroup darray-conversion-operators conversion operators **/
            ///@{

            ///@}
            /** @defgroup darray-sequence-methods **/
            ///@{

            ///@}
            /** @defgroup darray-printable-methods **/
            ///@{

            ///@}
            /** @defgroup darray-gcobject-methods **/
            ///@{

            ///@}

        private:
            /** @defgroup darray-instance-variables instance variables **/
            ///@{

            /** extent of @ref elts_ array **/
            size_type capacity_ = 0;
            /** array size
             *  Invariant: size_ <= capacity_
             **/
            size_type size_ = 0;
            /** array elements, using flexible array **/
            obj<AGCObject> elts_[];

            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DArray.hpp */
