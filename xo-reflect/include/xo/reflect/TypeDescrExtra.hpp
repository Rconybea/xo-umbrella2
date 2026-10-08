/* @file TypeDescrExtra.hpp */

#pragma once

#include "Metatype.hpp"
#include "Ownership.hpp"
#include <string>
/* note: this file #include'd into TypeDescr.hpp */
#include <cstdint>

namespace xo {
    namespace reflect {
        /* forward-declaring here.  see [reflect/struct/StructMember.hpp] */
        class StructMember;
        class FunctionTdxInfo;
        class EnumTdx;
        class StdAtomicTdx;
        class WrapperTdx;
        class LockableTdx;
        class TypeDescrBase;
        class TaggedPtr;

        /* information associated with a c++ type.
         * distinct from TypeDescrImpl:
         * 1. want to use reflection to support for runtime polymorphism over similar but
         *    not directly-related types:   for example
         *      std::vector<int>
         *    and
         *      std::list<std::string>
         *    are both ordered collections
         * 2. some information can't be universally established via template-fu,
         *    for example struct member names
         * 3. descriptions for recursive types require 2-stage construction
         *
         * A TypeDescrImpl instance will contain a pointer to a suitable
         * TypeDescrExtra instance.
         *
         * The single TypeDescrImpl instance for some type T can be established
         * automatically,  see Reflect::require().
         *
         * A specific TypeDescrExtra instance may be attached in a non-automated way
         * later
         */
        class TypeDescrExtra {
        public:
            using uint32_t = std::uint32_t;

        public:
            virtual ~TypeDescrExtra() = default;

            bool is_pointer() const { return this->metatype() == Metatype::mt_pointer; }
            bool is_vector() const { return this->metatype() == Metatype::mt_vector; }
            bool is_struct() const { return this->metatype() == Metatype::mt_struct; }
            bool is_function() const { return this->metatype() == Metatype::mt_function; }
            /** true iff a reflected enum (EnumReflector): metatype mt_atomic,
             *  with enumerator names in enum_info()
             **/
            bool is_enum() const { return this->enum_info() != nullptr; }
            /** true iff a std::atomic<T>: metatype mt_atomic, with a load
             *  and T's description in std_atomic_info()
             **/
            bool is_std_atomic() const { return this->std_atomic_info() != nullptr; }
            /** true iff a transparent wrapper (WrapperReflector): metatype
             *  mt_atomic, standing for one member it holds, wrapper_info()
             **/
            bool is_wrapper() const { return this->wrapper_info() != nullptr; }
            /** true iff a lockable (std::mutex, std::shared_mutex): metatype
             *  mt_atomic, with reader lock operations in lockable_info()
             **/
            bool is_lockable() const { return this->lockable_info() != nullptr; }

            virtual Metatype metatype() const = 0;
            /* given a T-instance,  report most-derived subtype of T to which *object belongs.
             * this works only for types that are derived from reflect::SelfTagging.
             */
            virtual TaggedPtr most_derived_self_tp(TypeDescrBase const * object_td, void * object) const;
            virtual uint32_t n_child(void * object) const = 0;
            /** number of children,  fixed at compile time.
             *  Will return 0 for types like std::vector<..> (because number is unknown);
             *  Will also return 0 for types like {bool, int, long} (because number is zero)
             **/
            virtual uint32_t n_child_fixed() const = 0;
            /** type description for i'th child,  based on information available at compile time.
             *  For vectors/pointers, this always refers to element type.
             *
             *  nullptr for atomics
             **/
            virtual const TypeDescrBase * fixed_child_td(uint32_t i) const = 0;
            virtual TaggedPtr child_tp(uint32_t i, void * object) const = 0;
            /* require:
             *   .is_struct()
             */
            virtual std::string const & struct_member_name(uint32_t i) const = 0;
            /* nullptr unless *this represents a struct/class type */
            virtual StructMember const * struct_member(uint32_t i) const;
            /** relationship between lifetime of an instance of this type,
             *  relative to lifetime of a child.
             **/
            virtual Ownership child_edge_ownership() const { return Ownership::owning; }

            // methods for working with guards (StructMemberDecl::guarded_by)

            /** number of distinct lockables guarding this struct's members;
             *  0 unless a reflected struct declared some
             **/
            virtual uint32_t n_guard() const { return 0; }
            /** lockable @p g, of the struct at @p object; its type has
             *  lockable_info().  require: g < n_guard()
             **/
            virtual TaggedPtr guard_tp(uint32_t g, void * object) const;

            // methods for working with reflected functions/methods

            /** number of arguments to function-like value
             *
             *  @pre @ref TypeDescrExtra::is_function() is true
             **/
            virtual const FunctionTdxInfo * fn_info() const { return nullptr; }
            virtual const TypeDescrBase * fn_retval() const { return nullptr; }
            virtual uint32_t n_fn_arg() const { return 0; }
            virtual const TypeDescrBase * fn_arg(uint32_t /*i_arg*/) const { return nullptr; }
            virtual bool fn_is_noexcept() const { return false; }

            // methods for working with reflected enums

            /** enumerators of a reflected enum (EnumReflector); nullptr
             *  for any other type, an enum not reflected included
             **/
            virtual const EnumTdx * enum_info() const { return nullptr; }

            // methods for working with std::atomic<T>

            /** load and value type of a std::atomic<T>; nullptr for any
             *  other type
             **/
            virtual const StdAtomicTdx * std_atomic_info() const { return nullptr; }

            // methods for working with transparent wrappers

            /** the wrapped member of a transparent wrapper
             *  (WrapperReflector); nullptr for any other type
             **/
            virtual const WrapperTdx * wrapper_info() const { return nullptr; }

            // methods for working with lockables

            /** reader lock operations of a lockable; nullptr for any other
             *  type
             **/
            virtual const LockableTdx * lockable_info() const { return nullptr; }
        }; /*TypeDescrExtra*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end TypeDescrExtra.hpp */
