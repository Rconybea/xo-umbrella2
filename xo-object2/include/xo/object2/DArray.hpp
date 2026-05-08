/** @file DArray.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/obj.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <concepts>
#include <cstdint>

namespace xo {
    namespace scm {
        class DArray;

        namespace detail {
            /** null base case **/
            static inline bool do_array_push_back(DArray *,
                                                  obj<xo::mm::AAllocator>)
            {
                return true;
            }

            template <typename A, typename... Rest>
            requires (std::convertible_to<A, obj<xo::mm::AGCObject>>)
            static bool do_array_push_back(DArray * lhs,
                                           obj<xo::mm::AAllocator> mm,
                                           A arg1,
                                           Rest... rest);
        }

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
            using AAllocator = xo::mm::AAllocator;
            //using ACollector = xo::mm::ACollector;
            using AGCObject = xo::mm::AGCObject;
            /** gc-centric object visitor **/
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            /** hint for object visitor **/
            using VisitReason = xo::mm::VisitReason;
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
            static DArray * _empty(obj<AAllocator> mm,
                                   size_type cap);

            /** ofp version of _empty(mm,cap) **/
            template <typename AFacet = AGCObject>
            static obj<AFacet,DArray> empty(obj<AAllocator> mm,
                                            size_type cap);

            /** create copy of @p src using memory from @p mm
             *  with capacity for @p new_cap elements
             **/
            static DArray * copy(obj<AAllocator> mm,
                                 DArray * src,
                                 size_type new_cap);

            /** create array containing elements @p args, using memory from @p mm.
             *  Nullptr if space exhausted.
             *
             *  Use:
             *    Darray * v = DArray::array(mm, e1, e2, e3);
             **/
            template <typename... Args>
                requires (std::convertible_to<Args, obj<AGCObject>> && ...)
            static DArray * array(obj<AAllocator> mm, Args... args);

            ///@}
            /** @defgroup darray-access access methods **/
            ///@{

            /** create fop for this instance  **/
            template <typename AFacet = AGCObject>
            obj<AFacet,DArray> ref() { return obj<AFacet,DArray>(this); }

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

            ///@}
            /** @defgroup darray-iterators iterators **/
            ///@{

            ///@}
            /** @defgroup darray-assign assignment **/
            ///@{

            /** store @p elt at position @p index.
             *  true on success, false otherwise
             **/
            bool assign_at(obj<AAllocator> mm, size_type index, obj<AGCObject> elt) noexcept;

            /** append @p elt at the end of array.
             *  true on success, false otherwise.
             *  on failure array is unaltered
             **/
            bool push_back(obj<AAllocator> mm, obj<AGCObject> elt) noexcept;

            template <typename... Args>
            requires (std::convertible_to<Args, obj<AGCObject>> && ...)
            bool push_back_all(obj<AAllocator> mm, Args... args) noexcept;

            /** store last element in array into @p elt and decrement array size.
             *  true on success; false on failure (implies array was empty)
             **/
            bool pop_back(obj<AGCObject> * p_elt = nullptr) noexcept;

            ///@}
            /** @defgroup darray-general general methods **/
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
            /** move to new address, mandated by @p gc **/
            DArray * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            /** forward elements to @p gc to-space; replace originals with forarding pointers **/
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;
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

        template <typename AFacet>
        obj<AFacet,DArray>
        DArray::empty(obj<AAllocator> mm, DArray::size_type cap)
        {
            DArray * retval = _empty(mm, cap);

            return obj<AFacet,DArray>(retval);
        }

        template <typename... Args>
            requires (std::convertible_to<Args, obj<DArray::AGCObject>> && ...)
        DArray *
        DArray::array(obj<AAllocator> mm, Args... args)
        {
            DArray * result = _empty(mm, sizeof...(args));
            if (result) {
                detail::do_array_push_back(result, mm, args...);
            }
            return result;
        }

        namespace detail {
            template <typename A, typename... Rest>
            requires (std::convertible_to<A, obj<xo::mm::AGCObject>>)
            static bool do_array_push_back(DArray * lhs,
                                           obj<xo::mm::AAllocator> mm,
                                           A arg1,
                                           Rest... rest)
            {
                return (lhs->push_back(mm, arg1)
                        && do_array_push_back(lhs, mm, rest...));
            }
        }

        template <typename... Args>
        requires (std::convertible_to<Args, obj<xo::mm::AGCObject>> && ...)
        bool
        DArray::push_back_all(obj<AAllocator> mm, Args... args) noexcept {
            return detail::do_array_push_back(this, mm, args...);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DArray.hpp */
