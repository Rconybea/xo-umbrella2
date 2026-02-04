/** @file facet_implementation.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "xo/facet/facet.hpp"
#include <concepts>
#include <type_traits>

namespace xo {
    namespace facet {
        // ----- facet implementation -----

        // An implementation provides behavior (i.e. code, not data)
        // for a particular abstract facet, specialized for a specific data type

        /** For example ISpecific = IComplex_DPolarCoords
         **/
        template <typename AFacet, typename ISpecific>
        concept implements_facet = requires {
            std::is_base_of_v<AFacet, ISpecific>;
            std::is_default_constructible_v<ISpecific>;
            std::is_standard_layout_v<ISpecific>;
            /** require no additional state **/
            sizeof(ISpecific) == sizeof(AFacet);
        };

        /** Use: when defining datatype recovery for a typed facet implementation:
         *
         *    template <typename Repr>
         *    struct AMyFacet_Impl : public AMyFacet {
         *        static void _something(Repr *);
         *        static int _andalso(Repr *, double somearg);
         *
         *        virtual void something(void * data) const final override {
         *           _something((Repr *)data);
         *        }
         *
         *        virtual int andalso(void * data, double somearg) const final override {
         *           _andalso((Repr *)data, somearg);
         *        }
         *
         *        static bool _valid;
         *    };
         *
         *    template <typename Repr>
         *    bool AMyFacet_Impl<Repr>::_valid
         *      = valid_facet_implementation<AMyFacet, AMyFacet_Specific>();
         **/
        template <typename AFacet, typename ISpecific>
        consteval bool valid_facet_implementation()
            requires (valid_abstract_facet<AFacet>())
        {
            static_assert(std::is_base_of_v<AFacet, ISpecific>,
                          "Facet implementation must inherit FacetRttiShim<AFacet>");
            static_assert(std::is_default_constructible_v<ISpecific>,
                          "Facet implementation must be default-constructible");
            static_assert(sizeof(ISpecific) == sizeof(AFacet),
                          "Facet implementation may not introduce state");
            static_assert(!std::has_virtual_destructor_v<ISpecific>,
                          "Facet implementation does not benefit from virtual dtor (since has no data)");
            static_assert(std::is_trivially_destructible_v<ISpecific>,
                          "Facet implementation expected to have trivial dtor (since has no data)");

            // don't need this test, it's covered by sizeof check
            //static_assert(std::is_pointer_interconvertible_base_of_v<AFacet, ISpecific>,
            //              "Interface implementation must directly inherit interface (no base offset)");

            return true;
        };

        /** Compile-time facet implementation lookup
         *
         *  @c FacetImplementation<AMyFacet, DSomeRepr>::ImplType
         *  gives the type that implements @c AMyFacet with state @c DSomeRepr
         *
         *    template<DRepr>
         *    struct FacetImplementation<AMyFacet, DRepr> {
         *        using ImplType = IMyFacet_Impl<DRepr>;
         *    };
         *
         *    template<>
         *    struct FacetImplementation<AMyFacet, DVariantPlaceholder> {
         *        using ImplType = IMyFacet_Any;
         *    };
         *
         **/
        template <typename AFacet, typename DRepr>
        struct FacetImplementation {
            //static_assert(false && "expect specialization <AFacet,DRepr> which should provide ImplType trait");
        };

        /** true iff FacetImplementation<AFacet,DRepr> has been specialized with ImplType.
         *  False when specialization header (IFacet_DRepr.hpp) not included.
         **/
        template <typename AFacet, typename DRepr>
        concept has_facet_impl = requires {
            typename FacetImplementation<AFacet, DRepr>::ImplType;
        };

        /** Retrieve facet implementation for a (facet,datatype) pair **/
        template <typename AFacet, typename DRepr>
        using FacetImplType = FacetImplementation<AFacet, DRepr>::ImplType;

        /** Use:
         *    auto iface = xo::facet::impl_for<AGCObject, DList>();
         *  if compiles, then iface is AGCObject interface with state DList.
         **/
        template <typename AFacet, typename DRepr>
        inline auto impl_for() {
            FacetImplType<AFacet, DRepr> iface;

            return iface;
        }

        /** Data type for facet implementation that supports runtime polymorphism.
         *  Implementation will stub all methods, since they will never be invoked.
         *
         *    template <>
         *    struct IMyFacet_Any : public AMyFacet {
         *        virtual void something(void * data) const final override { assert(false); }
         *        virtual int andalso(void * data) const final override { assert(false); return 0; }
         *
         *        static bool _valid;
         *    }
         *
         *    template <>
         *    bool IMyFacet_Any::_valid
         *      = valid_facet_implementation<AMyFacet, IMyFacet_Any>();
         **/
        struct DVariantPlaceholder {};
    } /*namespace facet*/
} /*namespace xo*/

/* end facet_implementation.hpp */
