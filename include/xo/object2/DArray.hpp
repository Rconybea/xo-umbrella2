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
#include <concepts>
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
        class DArray {
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

            /** create array containing elements @p args, using memory from @p mm.
             *  Nullptr if space exhausted.
             *
             *  Use:
             *    Darray * v = DArray::array(mm, e1, e2, e3);
             **/
            template <typename... Args>
                requires (std::same_as<Args, obj<AGCObject>> && ...)
            static DArray * array(obj<AAllocator> mm, Args... args);

            const obj<AGCObject> & operator[](size_type index) const noexcept { return elts_[index]; }
            obj<AGCObject> & operator[](size_type index) noexcept { return elts_[index]; }

            ///@}
            /** @defgroup darray-access acecss methods **/
            ///@{
            /** true iff array is empty **/
            bool is_empty() const noexcept { return size_ == 0; }
            /** only support finite arrays :-) **/
            bool is_finite() const noexcept { return true; }
            /** array capacity **/
            size_type capacity() const noexcept { return capacity_; }
            /** current array size (number of elements) **/
            size_type size() const noexcept { return size_; }
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

            /** pretty-printing support **/
            bool pretty(const ppindentinfo & ppii) const;

            ///@}
            /** @defgroup darray-gcobject-methods **/
            ///@{
            /** shallow memory consumption. Excludes child objects **/
            AAllocator::size_type shallow_size() const noexcept;
            /** return shallow copy of this array, using memory from @p mm **/
            DArray * shallow_copy(obj<AAllocator> mm) const noexcept;
            /** forward elements to @p gc to-space; replace originals with forarding pointers **/
            AAllocator::size_type forward_children(obj<ACollector> gc) noexcept;
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

        template <typename... Args>
            requires (std::same_as<Args, obj<DArray::AGCObject>> && ...)
        DArray *
        DArray::array(obj<AAllocator> mm, Args... args)
        {
            DArray * result = empty(mm, sizeof...(args));
            if (result) {
                (result->push_back(args), ...);
            }
            return result;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DArray.hpp */
