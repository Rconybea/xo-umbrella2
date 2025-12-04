/** @file IObject.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include <cstddef>
#include <cstdint>

namespace xo {
    namespace gc { class IAlloc; }

    /** @class IObject
     *  @brief Base interface for GC interaction
     *
     *  Classes that can be collected inherit this api
     *  indirectly via xo::Object (see xo/alloc/Object.hpp)
     **/
    class IObject {
    public:
        /** impl inheriting this class must provide gc hooks **/
        static constexpr bool _requires_gc_hooks = true;
        /** impl inheriting this class must use write barriers
         *  (so that GC allocator can remember cross-generational pointers)
         **/
        static constexpr bool _requires_write_barrier = true;

        /** GC write barrier:
         *  assign value @p rhs to member @p *lhs of @p parent.
         *  Identifiy and remember cross-generational pointers.
         **/
        template <typename T, typename Allocator>
        void _gc_assign_member(T ** lhs,
                               T * rhs,
                               Allocator & alloc)
        {
            static_assert(std::is_convertible_v<decltype(*lhs), IObject*>);

            alloc.mm_->assign_member(this, reinterpret_cast<IObject **>(lhs), rhs);
        }

        /** true iff this object represents a forwarding pointer.
         *  Forwarding pointers are exclusively created by the garbage collector;
         *  forwarding pointers (and only forwarding pointers) return true here.
         **/
        virtual bool _is_forwarded() const { return false; }

        /** offset for uncommon situation where pointer address is offset from object
         *  base address
         **/
        virtual IObject * _offset_destination(IObject * src) const { return src; };

        /** replace this object with a forwarding pointer referring to @p dest.
         **/
        virtual void _forward_to(IObject * dest) = 0;

        /** if this object represents a forwarding pointer, return its new location.
         *  forwarding pointers belong to the garbage collector implementation.
         *  (if you have to ask -- no, your class is not a forwarding pointer)
         *  all other objects return nullptr here.
         **/
        virtual IObject * _destination() { return nullptr; }

        /** return amount of storage (including padding) consumed by this object,
         *  excluding immediate Object-pointer children
         **/
        virtual std::size_t _shallow_size() const = 0;

        /** if subject is allocated by GC:
         *  - create copy C in to-space
         *  - destination C will be nursery|tenured depending on location of this.
         *  else
         *  - return this to disengage from GC
         *
         *  Require: @ref mm is an instance of @ref gc::GC
         **/
        virtual IObject * _shallow_copy(gc::IAlloc * gc) const = 0;

        /** update child pointers that refer to forwarding pointers,
         *  replacing them with the correct destination.
         *  See @ref Object::deep_move
         *
         *  this   gray object, located in to-space.
         *  fwd1   forwarding objects.
         *         Located in from-space. Invalid at end of GC cycle.
         *  p1,p2  source pointers.
         *  D1,D2  already-forwarded objects. located in to-space.
         *
         *  before:
         *    this       fwd1
         *    +----+     +-+
         *    | p1 ----->|x|-------> D1
         *    |    |     +-+
         *    |    |
         *    | p2 ----------------> D2
         *    +----+
         *
         *  after:
         *    this
         *    +----+
         *    | p1 ----------------> D1
         *    |    |
         *    |    |
         *    | p2 ----------------> D2
         *    +----+
         *
         *  this is now white
         *
         *  @return shallow size of *this.  Must exactly match the amount of memory in to-space
         *  allocated by @ref _shallow_move
         *
         **/
        virtual std::size_t _forward_children(gc::IAlloc * gc) = 0;
    };

    static_assert(std::is_destructible_v<IObject>, "IObject must be destructible");

    /** @class Cpof
     *  @brief argument to operator new used for garbage collector evacuation phase
     *
     *  Tag overloaded operator new to activate allocation policy based on location
     *  in memory of source object.
     **/
    class Cpof {
    public:
        explicit Cpof(gc::IAlloc * mm, const IObject * src) : mm_{mm}, src_{src} {}

        gc::IAlloc * mm_ = nullptr;
        const void * src_ = nullptr;
    };
}

/* end IObject.hpp */
