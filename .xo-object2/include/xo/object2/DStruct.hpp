/** @file DStruct.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DArray.hpp"
#include <xo/gc/GCObject.hpp>
#include <xo/gc/Collector.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/obj.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <concepts>
#include <cstdint>

namespace xo {
    namespace scm {
        /** @class DStruct
         *  @brief Polymorphic in-memory key-value store with gc hooks
         *
         *  Small dictionary implementation for Schematika.
         *  O(n) lookup, at least for now.  Keys are unique strings.
         *  Intended to have key-space fixed at comptime.
         *  Relevant since keys are immortal DUniqueStrings.
         **/
        class DStruct {
        public:
            /** @defgroup dstruct-types type traits **/
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
            /** @defgroup dstruct-ctors constructors **/
            ///@{

            /** default ctor. zero capacity sentinel **/
            DStruct() = default;

            /** not simply copyable because of flexible array.
             *  Need allocator. See @ref clone
             **/
            DStruct(const DStruct &) = delete;

            /** create empty array with space for @p cap elements
             *  using memory from allocator @p mm.
             *  Nullptr if space exhausted
             **/
            static DStruct * empty(obj<AAllocator> mm,
                                  size_type cap);

            /** create copy of @p src using memory from @p mm
             *  with capacity for @p new_cap elements
             **/
            static DStruct * copy(obj<AAllocator> mm,
                                 DStruct * src,
                                 size_type new_cap);

            /** create array containing elements @p args, using memory from @p mm.
             *  Nullptr if space exhausted.
             *
             *  Use:
             *    Dstruct * v = DStruct::array(mm, e1, e2, e3);
             **/
            template <typename... Args>
                requires (std::same_as<Args, obj<AGCObject>> && ...)
            static DStruct * array(obj<AAllocator> mm, Args... args);

            ///@}
            /** @defgroup dstruct-access acecss methods **/
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

            const obj<AGCObject> & operator[](size_type index) const noexcept { return elts_[index]; }
            obj<AGCObject> & operator[](size_type index) noexcept { return elts_[index]; }

            ///@}
            /** @defgroup dstruct-iterators iterators **/
            ///@{

            ///@}
            /** @defgroup dstruct-assign assignment **/
            ///@{
            /** append @p elt at the end of array.
             *  true on success, false otherwise
             **/
            bool push_back(obj<AGCObject> elt) noexcept;

            ///@}
            /** @defgroup dstruct-general general methods **/
            ///@{

            /** resize to @p new_size.  @p new_size may not be larger than capacity
             *  Return true if resize was accomplished; false otherwise.
             **/
            bool resize(size_type new_size) noexcept;

            /** reduce array capacity to current array size
             *
             *  note: with X1Collector, capacity is reduced but memory not recycled
             *        until next collection
             **/
            void shrink_to_fit() noexcept;

            ///@}
            /** @defgroup dstruct-conversion-operators conversion operators **/
            ///@{

            ///@}
            /** @defgroup dstruct-sequence-methods **/
            ///@{

            ///@}
            /** @defgroup dstruct-printable-methods **/
            ///@{

            /** pretty-printing support **/
            bool pretty(const ppindentinfo & ppii) const;

            ///@}
            /** @defgroup dstruct-gcobject-methods **/
            ///@{
            /** shallow memory consumption. Excludes child objects **/
            AAllocator::size_type shallow_size() const noexcept;
            /** forward elements to @p gc to-space; replace originals with forarding pointers **/
            AAllocator::size_type forward_children(obj<ACollector> gc) noexcept;
            ///@}

        private:
            /** @defgroup dstruct-instance-variables instance variables **/
            ///@{

            /** extent of @ref elts_ array **/
            size_type capacity_ = 0;
            /** array size
             *  Invariant: size_ <= capacity_
             **/
            size_type size_ = 0;
            /** struct keys.  These will be unique strings **/
            DArray * keys_ = nullptr;
            /** struct member values **/
            DArray * values_ = nullptr;

            ///@}
        };

        template <typename... Args>
            requires (std::same_as<Args, obj<DStruct::AGCObject>> && ...)
        DStruct *
        DStruct::array(obj<AAllocator> mm, Args... args)
        {
            DStruct * result = empty(mm, sizeof...(args));
            if (result) {
                (result->push_back(args), ...);
            }
            return result;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DStruct.hpp */
