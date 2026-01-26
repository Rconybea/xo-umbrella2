/** @file GCObjectConversion.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/gc/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/facet/obj.hpp>

namespace xo {
    namespace scm {
        /** @brief compile-time conversion obj<AGCObject> <-> T
         *
         *  Specialize for each T that participates in conversion.
         *  Methods here aren't implemented
         **/
        template <typename T>
        struct GCObjectConversion {
            using AGCObject = xo::mm::AGCObject;
            using AAllocator = xo::mm::AAllocator;

            /** find gc-aware representation for @p x.
             *  If necessary allocate from @p mm, but may
             *  refer to @p x in-place
             **/
            static obj<AGCObject> to_gco(obj<AAllocator> mm, const T & x);
            /** convert to native representation @tparam T from gc-aware
             *  @p gco.  If necessary allocate from @p mm, but
             *  may instead refer to @p x in-place
             **/
            static T from_gco(obj<AAllocator> mm, obj<AGCObject> gco);
        };

        /** Motivating use-case for GCObjectConversion is to transform
         *  primitive function arguments and results to/from gc-aware
         *  representation.
         *
         *  However: Schematika also supports runtime polymorphism
         *  which leads to primitives that expect obj<AFacet> arguments.
         *
         *  Also, Schematika expression parser needs representation for
         *  expressions, before type unification.
         *
         *  Consider a function like:
         *    def fact = lambda (n : i64) { if (n <= 0) then 1 else (n * fact(n - 1)); }
         *  During expression parsing the rhs argument to multiply has unknown type.
         *  To construct an expression for input to unification will use polymorphic
         *  binding for multiply primitive, relying on specialization here for
         *  its implementation.
         **/
        template <typename AFacet, typename DRepr>
        struct GCObjectConversion<obj<AFacet,DRepr>> {
            using AGCObject = xo::mm::AGCObject;
            using AAllocator = xo::mm::AAllocator;
            using FacetRegistry = xo::facet::FacetRegistry;
            using DVariantPlaceholder = xo::facet::DVariantPlaceholder;

            static obj<AGCObject> to_gco(obj<AAllocator>,
                                         obj<AFacet,DRepr> gco) {
                if constexpr (std::is_same_v<AFacet, AGCObject>) {
                    // trivial conversion!
                    return gco;
                } else if constexpr (std::is_same_v<DRepr, DVariantPlaceholder>) {
                    // runtime polymorphism
                    return FacetRegistry::instance().variant<AGCObject,AFacet>(gco);
                } else /* DRepr != DVariantPlaceholder */ {
                    // known content w/ fat object pointer
                    return obj<AGCObject,DRepr>(gco.data());
                }
            }

            static obj<AFacet,DRepr> from_gco(obj<AAllocator>,
                                              obj<AGCObject> gco) {
                if constexpr (std::is_same_v<AFacet, AGCObject>) {
                    // trivial conversion
                    return gco;
                } else {
                    // both runtime and comptime polymorphism
                    // use same path here, since representation of @p gco
                    // is type-erased here

                    return FacetRegistry::instance().variant<AFacet,AGCObject>(gco);
                }
            }
        };
    } /*namespace scm */
} /*namespace xo*/

/* end GCObjectConversion.hpp */
