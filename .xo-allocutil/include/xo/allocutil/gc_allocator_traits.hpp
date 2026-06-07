/** @file gc_allocator_traits.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include <type_traits>
#include <memory>
#include <cassert>

namespace xo {
    class IObject;

    namespace gc {
        class IAlloc;

        /** object interface for allocators A that don't provide A::gc_object_interface.
         *  See gc_allocator_traits<A>
         **/
        struct FallbackObjectInterface {
            /** see also IObject::_requires_gc_hooks **/
            static constexpr bool _requires_gc_hooks = false;
            /** see also IObject::_requires_write_barrier **/
            static constexpr bool _requires_write_barrier = false;

            /** see also IObject::_gc_assign_member **/
            template <typename T, typename AA>
            void _gc_assign_member(T ** lhs,
                                   T * rhs,
                                   AA & alloc) {
                (void)alloc;
                *lhs = rhs;
            }

            virtual void display(std::ostream &) const {}
            virtual bool _is_forwarded() const { return false; }
            virtual std::size_t _shallow_size() const { assert(false); return 0; }
            virtual IObject * _shallow_copy(gc::IAlloc *) const { assert(false); return nullptr; }
            virtual std::size_t _forward_children(gc::IAlloc *) { assert(false); return 0; }
        };

        /** dummy GC interface.
         *  non-empty intersection with IAlloc
         **/
        template <typename GcObjectInterface>
        struct FallbackGcInterface {
            template <typename Allocator>
            FallbackGcInterface(Allocator & /*alloc*/) {}

            bool check_write_barrier(const void * /*parent*/,
                                     const void * const * /*lhs*/,
                                     bool /*may_throw*/) { return true; };
        };

        /** Extended version of
         *    std::allocator_traits<Allocator>
         *  Introduces additional i/face methods
         *  for garbage-collector-enabled allocators
         *
         *  Use Cases:
         *  1. drop-in replacement for std::allocator_traits<Allocator>
         *     with non-gc-aware allocators.
         *  2. allows a gc-aware template class to activate
         *     gc support when used with a collecting allocator
         *     (i.e. xo::gc::allocator<xo::gc::GC>)
         *  3. allows a gc-aware template class T to fallback
         *     to ordinary allocator-aware behavior for non-gc
         *     allocators, such as std::allocator<T>,
         *     but also pool allocators etc.
         *
         *  An allocator A can identify itself as a copying collector:
         *
         *  1. provide A::object_interface
         *     per-object header interface: tells garbage collector
         *     how to navigate object graph.
         *       A::gc_object_interface = xo::IObject
         *     contains virtual methods; classes that can be garbage
         *     collected should inherit this interface
         *
         *  2. provide A::has_incremental_gc_interface
         *       A::has_incremental_gc_interface = std::true_type
         *     This doesn't imply A is a garbage-collecting allocator;
         *     it just implies that it supports a collection api.
         *     - xo::gc::ArenaAlloc has a collection API, but does not
         *       provide garbage collection
         *     - xo::gc::GC has a collection API and also provides
         *       garbage collection
         *
         *     GC object model
         *     2a. A GC-allocated object is an object that GC manages
         *         atomically. All memory associated with a GC-allocated
         *         object has the same lifetime.
         *     2b. A GC-allocation is 1:1 with a GC-allocated object
         *     2c. A GC-allocated object may have internal pointers.
         *         These are pointer interior to the same original
         *         allocation. It's the responsibility of the object to update these
         *         (if/when GC moves said object) via GC hooks.
         *     2d. A GC-allocated object may have external pointers
         *         to other GC-allocated objects. Managing these is split
         *         between GC and object itself. GC takes responsibility
         *         for moving the destination objects.
         *         Object is responsible for telling GC about such pointers
         *         and changes to their values
         *         (e.g. IObject::_forward_children())
         *
         *     GC object implementation: gc objects must:
         *     2a. inherit A::object_interface
         *     2b. implement A::object_interface::_shallow_size()
         *     2c. implement A::object_interface::_shallow_copy(alloc)
         *     2d. implement A::object_interface::_forward_children(alloc)
         *     in multiple inheritance scenarios
         *     2e. implement A::object_interface::_offset_destination(src)
         *
         *  3. write barrier support:
         *     A generational GC needs to track changes that create or modify
         *     inter-generational pointers.
         *
         *     GC-aware classes could write:
         *       MyClass::update_pointer_state(IObject *new_value, gc::IAlloc *gc) {
         *         if constexpr (GcObjectInterface::_requires_write_barrier) {
         *           gc->assign_member(this, &some_member_, new_value);
         *         } else {
         *           this->some_member_ = new_value;
         *         }
         *       }
         *
         *     but simpler:
         *       GcObjectInterface::_gc_assign_member(this, &some_member_, new_value, alloc_);
         *
         *  Design Notes:
         *  - virtual-method choice requires vtable pointer per object;
         *    but zero *marginal* space cost for types that would have
         *    a vtable pointer anyway.
         *  - can still handle non-vtable objects, by providing a
         *    object-interface-inheriting wrapper.
         *  - less-intrusive (though less space-efficient) alternative
         *    would be to use a type-registration system;
         *    then GC hooks could be setup independently of a subject type.
         *    (watch out for pimpl implementations though!)
         **/
        template <typename Allocator>
        struct gc_allocator_traits : std::allocator_traits<Allocator> {
            using super = std::allocator_traits<Allocator>;
            using pointer = typename super::pointer;
            using value_type = typename super::value_type;
            using super::construct;
            using super::destroy;
            using super::allocate;
            using super::deallocate;

            // ----------------------------------------------------------------

            // default: allocator A fallback to standard non-gc allocator behavior
            template <typename A, typename = void>
            struct has_incremental_gc_interface : std::false_type {};

            // opt-in: A provides nested type 'has_incremental_collector_interface':
            // struct A {
            //   using has_incremental_collector = std::true_type;
            // };
            template <typename A>
            struct has_incremental_gc_interface<A, std::void_t<typename A::has_incremental_gc_interface>> :
                A::has_incremental_gc_interface {};

            /** true iff this allocator advertises itself as an incremental collector.
             *  Allocator will include:
             *
             *  struct IAlloc {
             *    using has_incremental_gc_interface = std::true_type;
             *  };
             **/
            static inline constexpr
            bool
            has_incremental_gc_interface_v = has_incremental_gc_interface<Allocator>::value;

            // ----------------------------------------------------------------

            // default: allocate A fallback to standard non-GC allocator behavior
            template <typename A, typename = void>
            struct has_trivial_deallocate : std::false_type {};

            // opt-in: A provides nested type 'has_trivial_deallocate':
            // struct A {
            //   using has_trivial_deallocate = std::true_type;
            // };
            template <typename A>
            struct has_trivial_deallocate<A, std::void_t<typename A::has_trivial_deallocate>> :
                A::has_trivial_deallocate {};

            /** true iff this allocator advertises trivial deallocate
             *  Allocate will include:
             *
             *  struct IAlloc {
             *    using has_trivial_deallocate = std::true_type;
             *  };
             **/
            static inline constexpr
            bool
            has_trivial_deallocate_v = has_trivial_deallocate<Allocator>::value;

            // ----------------------------------------------------------------

            // default: empty object interface.
            //
            // classes that want to conditionally support GC
            // (e.g. see xo::tree::RedBlackTree, xo::tree::Node
            //       in xo-ordinal-tree)
            // can inherit
            //   gc_allocator_traits<Allocator>::template object_interface<Allocator>
            //
            template <typename A, typename = void>
            struct object_interface : public FallbackObjectInterface {};

            // specialization when an allocator A
            // (which will actuallly be Allocator via SFINAE)
            // provides gc_object_interface
            //
            template <typename A>
            struct object_interface<A, std::void_t<typename A::gc_object_interface>>
                : public A::gc_object_interface {};

            // classes that want to conditionally support GC
            // (e.g. see xo::tree::RedBlackTree, xo::tree::Node
            //       in xo-ordinal-tree)
            // can inherit
            //   gc_allocator_traits<Allocator>::object_interface_type
            //
            using object_interface_type = object_interface<Allocator>;

            // ----------------------------------------------------------------

            // default: minimal garbage collector interface.
            //
            // Use in allocator-aware components that need conditionally
            // to engage with GC functionality.
            // For example in RedBlackTree::verify_ok() want to check
            // cross-generational pointers.
            //
            // gc_interface gc
            //   - gc_interface(A & alloc)
            //   - gc.check_write_barrier(const object_interface_type * p,
            //                            const object_interface_type * const * lhs,
            //                            bool may_throw)
            //
            template <typename A, typename = void>
            struct gc_interface : public FallbackGcInterface<object_interface_type> {};

            // allocator opt-in by providing a gc_interface type
            template <typename A>
            struct gc_interface<A, std::void_t<typename A::gc_interface>> : public A::gc_interface {};

            // interface for (narrow) GC interaction.
            // Construct from allocator
            using gc_interface_type = gc_interface<Allocator>;

        };
    } /*namespace gc*/
} /*namespace xo*/

/* end gc_allocator_traits.hpp */
