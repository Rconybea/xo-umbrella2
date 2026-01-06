/** @file OObject.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "facet_implementation.hpp"
#include <xo/reflectutil/typeseq.hpp>
#include <new>
#include <cstring>
#include <cstddef>

namespace xo {
    namespace facet {
        template <typename OObject>
        consteval bool valid_object_traits()
        {
            static_assert(requires { typename OObject::AbstractInterface; },
                              "OObject type must provide typename Object::AbstractInterface");
            static_assert(requires { typename OObject::ISpecific; },
                              "OObject type must provide typename Object::ISpecific");
            static_assert(requires { typename OObject::DataType; },
                              "OObject type must provide typename Object::DataType");
            static_assert(valid_facet_implementation<OObject::AbstractInterface, OObject::ISpecific>,
                              "OObject::ISpecific must implement Object::AbstractInterface");
            static_assert(std::is_standard_layout_v<OObject>,
                              "OObject must have standard layout, i.e. no virtual methods."
                              " Virtual methods belong in OObject::AbstractInterface");
            static_assert(requires(const OObject & obj) {
                               { obj.iface() } -> std::convertible_to<const typename OObject::AbstractInterface*>; },
                              "OObject must have non-virtual method iface()"
                              " returning const OObject::AbstractInterface");
            static_assert(requires(const OObject & obj) {
                              { obj.data() } -> std::convertible_to<typename OObject::DataType*>; },
                              "OObject must have non-virtual method data() returning OObject::DataType*");

            return true;
        }

        /** A "fat object pointer": combines two pointers:
         *
         *  1. behavior: an interface pointer
         *     (implementation of @tparam AFacet a.k.a. vtable pointer)
         *     for passive state DRepr.
         *     Interface pointers are static globals.
         *
         *  2. state: a data pointer to instance of passive state DRepr
         *     An OObject instance does not own its data pointer
         *
         *  Performance note: when DRepr can be determined at compile time,
         *  it's often feasible to optimize away the interface part.
         *
         *  Runtime polymorphism when @tparam DRepr is @ref DVariantPlaceholder
         *
         *  Application code should not use this directly.
         *  Instead, inherit a facet-specific routing wrapper that automatically
         *  injects @ref data as first argument to @ref iface_ methods.
         **/
        template <typename AFacet, typename DRepr = DVariantPlaceholder>
        struct OObject {
            using FacetType = AFacet;
            using ISpecific = FacetImplType<AFacet, DRepr>;
            using DataType = DRepr;
            using DataPtr = DRepr*;
            using Opaque = void *;
            using typeseq = xo::reflect::typeseq;

            /* required for vtable swapping to work */
            //static_assert(std::is_trivially_copyable_v<AFacet>);

            explicit OObject() {
                ISpecific tmp;
                memcpy(&(iface_[0]), (void*)&tmp, sizeof(ISpecific));
            }
            explicit OObject(DataPtr d) : data_{d} {
                ISpecific tmp;
                memcpy(&(iface_[0]), (void*)&tmp, sizeof(ISpecific));
            }

            OObject(const OObject & oother) {
                _launder_from(oother);
            }
            OObject(OObject && oother) {
                _launder_from(oother);
            }

            /** trivial: nothing to do for @ref iface_ and does not own @ref data_ **/
            ~OObject() = default;

            /** OObject is truthy **/
            operator bool() const { return data_ != nullptr; }

            // ----- iface() for variant fat pointer -----

            /** interface pointer for variant OObject instances.
             *  These instance support runtime polymorphism.
             **/
            const FacetType * iface() const
                requires std::is_same_v<DataType, DVariantPlaceholder>
            {
                /* std::launder:
                 *
                 * contents of iface_ at runtime will not match
                 * compile-time datatype. This prohibits compiler de-virtualizing
                 * calls to ISpecific methods, based on mistaken belief that
                 * vtable pointer is known at compile time.
                 */
                return std::launder((FacetType *)(&iface_[0]));
            }

            /** non-const verison. Technically all interface methods are const.
             *  But counterintuitive to have to mark interface methods const
             *  that are dedicated to mutable data.
             **/
            FacetType * iface()
                requires std::is_same_v<DataType, DVariantPlaceholder>
            {
                return std::launder((FacetType *)(&iface_[0]));
            }

            // ----- iface() for typed fat pointer -----

            /** interface pointer for OObject instance with representation
             *  known at compile time.
             *
             *  Calls here should be straightforward to devirtualize
             **/
            const FacetType * iface() const
                requires (!std::is_same_v<DataType, DVariantPlaceholder>)
            {
                /* don't use std::launder: want compiler to devirtualize
                 * calls to virtual @ref iface_ methods
                 */
                return std::launder((FacetType *)(&iface_[0]));
            }

            /** non-const verison. Technically all interface methods are const.
             *  But counterintuitive to have to mark interface methods const
             *  that are dedicated to mutable data.
             **/
            FacetType * iface()
                requires(!std::is_same_v<DataType, DVariantPlaceholder>)
            {
                return std::launder((FacetType *)&(iface_[0]));
            }

            DataPtr data() const { return data_; }
            Opaque opaque_data() const { return data_; }

            void reset() { data_ = nullptr; }
            void reset_opaque(Opaque data) { data_ = (DataPtr)data; }

            template <typename DOther>
            OObject & from_obj(const OObject<AFacet, DOther> & oother) {
                if constexpr (std::is_same_v<DRepr, DVariantPlaceholder>) {
                    /* Actual runtime type of other encoded in other.iface()
                     * (whether or not DOther says other is variant).
                     * Either way need to force vtable replacement, hence memcpy here
                     */
                    _launder_from(oother);
                } else if constexpr (std::is_convertible_v<DRepr, DOther>) {
                    /* other is typed, consistently with *this */
                    this->from_data(oother.data());
                } else
                {
                    // downcast from variant must be explicit
                    //  + may fail at runtime

                    static_assert(std::is_same_v<DRepr, DVariantPlaceholder>
                                  || std::is_convertible_v<DRepr, DOther>);
                }
                return *this;
            }

            /**
             *  We're either:
             *  - assigning from pointer with compatible representation
             *  - implementing the fat-object-pointer equivalent of
             *    assigning a derived pointer to a base pointer.
             **/
            template <typename DOther>
            OObject & from_data(DOther * other) {
                static_assert(std::is_same_v<DRepr, DVariantPlaceholder>
                              || std::is_convertible_v<DOther*, DRepr*>);

                if constexpr (!std::is_same_v<DRepr, DVariantPlaceholder>
                              && std::is_convertible_v<DOther*, DRepr*>)
                {
                    /* assigning to typed data, from something with consistent
                     * representation keep .iface_ pointer
                     */
                    this->data_ = other;
                } else /*DRepr is DVariantPlaceholder*/ {
                    /** assigning to variant
                     *
                     *  This implementation only valid for POD pointers.
                     **/

                    /* acquire fat object pointer for (AFacet, DOther) */
                    OObject<AFacet, DOther> oother(other);

                    static_assert(sizeof(*this) == sizeof(oother));

                    _launder_from(oother);
                }

                return *this;
            }

            /**
             *  Downcast to pointer of type DOther*, if valid.
             *  Provided when actual type of @ref data_ is not DRepr,
             *  because DRepr is DVariantPlaceholder.
             *
             *  We can't rely on dynamic_cast here, because DRepr's
             *  don't need to be related as far as c++ type system is
             *  concerned.
             **/
            template <typename DOther>
            DOther * downcast() const
                requires (std::is_same_v<DataType, DVariantPlaceholder>)
            {
                if (data_ && (typeseq::id<DOther>() == this->iface()->_typeseq())) {
                    /* actual runtime type for data_ is DOther,
                     * safe to reinterpret
                     */
                    return reinterpret_cast<DOther*>(data_);
                } else {
                    return nullptr;
                }
            }

            template <typename DOther>
            void _launder_from(const OObject<AFacet, DOther> & oother) {
                ::memcpy((void*)this, &oother, sizeof(*this));
                //iface_ = *std::launder(&iface_);
            }

#ifdef NOPE
            DRepr & operator*() { return *data_; }
#endif

            OObject & operator=(const OObject & oother) {
                if (this != &oother) {
                    _launder_from(oother);
                }
                return *this;
            }

            OObject & operator=(OObject && oother) {
                if (this != &oother) {
                    _launder_from(oother);
                }
                return *this;
            }

#ifdef NOT_IN_USE
            // not sure if this is a good idea. could just as well write
            //   auto obj = ...;
            //   *obj.data() == rhs

            /** assign contents of rhs in-place **/
            OObject & operator=(const DRect & rhs) {
                assert(data_);

                *(this->data_) = rhs;

                return *this;
            }
#endif

            /** fetch data pointer.  load-bearing for routing classes **/
            static bool _valid;

            /** runtime interface for this object.
             *  use byte array to make sure compiler doesn't get clever
             *  (but mistaken) ideas
             **/
            alignas(ISpecific) std::byte iface_[sizeof(ISpecific)]; //ISpecific iface_;

            /** runtime state for this object **/
            DataPtr data_ = nullptr;
        };

        template <typename AFacet, typename DRepr>
        bool
        OObject<AFacet, DRepr>::_valid = valid_object_traits<OObject>();

    } /*namespace facet*/
} /*namespace xo*/

/* end OObject.hpp */
