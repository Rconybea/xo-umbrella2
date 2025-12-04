/** @file gc_allocator_traits.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include <type_traits>
#include <memory>

namespace xo {
    namespace gc {
        /** Extended version of
         *    std::allocator_traits<Allocator>
         *  Introduces additional i/face methods
         *  for garbage-collector-enabled allocators
         *
         *  allocator A can identify itself as a copying collector:
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
         *     GC-allocated objects must:
         *     2a. inherit A::object_interface
         *     2b. implement A::object_interface::_shallow_size()
         *     2c. implement A::object_interface::_shallow_copy(alloc)
         *     2d. implement A::object_interface::_forward_children(alloc)
         *     in multiple inheritance scenarios
         *     2e. implement A::object_interface::_offset_destination(src)
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

            // default: allocator A fallback to standard non-gc allocator behavior
            template <typename A, typename = void>
            struct has_incremental_gc_interface : std::false_type {};

            // opt-in: A provides nested type 'has_incremental_collector_interface':
            // struct A {
            //   using is_incremental_collector = std::true_type;
            // };
            template <typename A>
            struct has_incremental_gc_interface<A, std::void_t<typename A::has_incremental_gc_interface>> :
                A::has_incremental_gc_interface {};

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

            // default: empty object interface.
            //
            // classes that want to conditionally support GC
            // (e.g. see xo::tree::RedBlackTree, xo::tree::Node
            //       in xo-ordinal-tree)
            // can inherit
            //   gc_allocator_traits<Allocator>::template object_interface<Allocator>
            //
            template <typename A, typename = void>
            struct object_interface {
                /** see also IObject::_requires_gc_hooks **/
                static constexpr bool _requires_gc_hooks = false;
                /** see also IObject::_requires_write_barrier **/
                static constexpr bool _requires_write_barrier = false;

                /** see also IObject::_gc_assign_member **/
                template <typename T>
                void _gc_assign_member(T ** lhs,
                                       T * rhs,
                                       A & alloc)
                {
                    *lhs = rhs;
                }

            };

            // specialization when A provides gc_object_interface
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

            /** true iff this allocator advertises itself as an incremental collector.
             *  Allocator will include:
             *
             *  struct IAlloc {
             *    using has_incremental_gc_interface = std::true_type;
             *  };
             **/
            static inline constexpr bool has_incremental_gc_interface_v = has_incremental_gc_interface<Allocator>::value;

            /** true iff this allocator advertises trivial deallocate
             *  Allocate will include:
             *
             *  struct IAlloc {
             *    using has_trivial_deallocate = std::true_type;
             *  };
             **/
            static inline constexpr bool has_trivial_deallocate_v = has_trivial_deallocate<Allocator>::value;
        };
    } /*namespace gc*/
} /*namespace xo*/

/* end gc_allocator_traits.hpp */
