/** @file obj.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "RRouter.hpp"
#include <utility>
#include <cassert>

namespace xo {
    namespace facet {
        /** object with borrowed state pointer
         *  - With default DRepr argument:
         *    type-erased polymorphic container
         *  - with specific DRepr argument:
         *    typed container. Trivially de-virtualizable
         *
         *  Example:
         *    std::unique_ptr<DRectCoords> z1_in
         *      = std::make_unique<DRectCoords>(1.0, 0.0):
         *    ubox<AComplex> z1{z1_in.release()};
         *    z1.xcoord();
         *
         *
         *            +-----+             +-----------------+
         *  Interface |   x-------------->| vtable for      |
         *            +-----+             | some descendant |
         *       Data |   x--------\      | of AInterface   |
         *            +-----+      |      |                 |
         *                         |      +-----------------+
         *                         |
         *                         |      +--------------+
         *                         \----->| data :: Repr |
         *                                +--------------+
         *
         *  Binary representation of unay<AInterface, Data>
         *  is compatible for different values of @tparam Data
         *  as long as vtable pointer moves along with data pointer.
         *
         *  In particular binary representation for
         *  ubox<AInterface,D> is as if it inherited ubox<AInterface>
         *  (even though it does not as far as compiler is concerned)
         *
         *  This is load-bearing for @ref move2any see below
         **/
        template <typename AFacet, typename DRepr = DVariantPlaceholder>
        struct obj : public RoutingType<AFacet, OObject<AFacet, DRepr>> {
            using Super = RoutingType<AFacet, OObject<AFacet, DRepr>>;

            obj() : Super() {}
            explicit obj(Super::DataPtr d) : Super(d) {}

            obj(const obj & rhs) = default;

            /** Runtime polymorphism:
             *  assemble variant from specific interface @p iface
             *  and type-erased representation @p data.
             *
             *  Implements
             *    obj<AFacet>::variant(iface, data)
             **/
            obj(const AFacet * iface, void * data)
                requires std::is_same_v<DRepr, DVariantPlaceholder>
            : Super(iface, data)
                {}

            /** pseudo copy constructor
             *
             *  Intended for use cases:
             *    obj<AFoo, DRepr> lhs = obj<AFoo, DRepr>   // same type on rhs
             *    obj<AFoo> lhs = obj<AFoo, DRepr>          // variant lhs, typed rhs
             *    obj<Afoo> lhs = obj<AFoo>                 // variant on both sides
             **/
            template <typename DOther>
            obj(const obj<AFacet, DOther> & other)
                requires (std::is_same_v<DRepr, DVariantPlaceholder>
                          || std::is_convertible_v<DRepr, DOther>)
                : Super()
            {
                this->from_obj(other);
            }

            /** move constructor from a different representation.
             *  allowed given:
             *  - same abstract interface
             *  - same strategy for holding state (naked / unique / refcounted ...)
             **/
            template <typename DOther>
            obj(obj<AFacet, DOther> && other)
                    requires (std::is_same_v<DRepr, DVariantPlaceholder>
                              || std::is_convertible_v<DRepr, DOther>)
              : Super()
            {
                /* replacing .iface_ along w/ .data_ */
                this->from_obj(other);
            }

            obj & operator=(const obj & rhs) {
                /* ensure we replace .iface_ along w/ .data_ */
                this->from_obj(rhs);
                return *this;
            }

            /** safe downcast from variant. null if downcast fails
             *
             *  Use:
             *    obj<AFoo> x = ...;
             *    obj<AFoo,DQuux> quux = obj<AFoo,DQuux>::from(x);
             **/
            static obj from(const OObject<AFacet> & other) {
                return obj(other.template downcast<DRepr>());
            }

            /** Runtime polymorphism.
             *  Create variant given interface @p iface,
             *  type-erased represention @p data
             *
             *  Use:
             *    AFoo * impl = ....;
             *    auto x = obj<AFoo>::variant(impl, data)
             **/
            static obj variant(const AFacet * iface, void * data)
                requires std::is_same_v<DRepr, DVariantPlaceholder>
                {
                    return obj(iface, data);
                }

            /** enabled when RRouter<AFacet> provides _preincrement.
             *  Note we don't need this trick for comparison operators,
             *  since return type is fixed.
             *
             *  For example see comparison overloads in RAllocIterator.hpp
             **/
            obj & operator++() noexcept { this->_preincrement(); return *this; }
        };

        template <typename AFacet>
        using vt = obj<AFacet, DVariantPlaceholder>;

        /** Use:
         *    auto o = with_facet<AAllocator>::mkobj(&data);
         **/
        template <typename AFacet>
        struct with_facet {
            template <typename DRepr>
            static obj<AFacet, DRepr> mkobj(DRepr * data) { obj<AFacet, DRepr> x(data); return x; }
        };

    } /*namespace facet*/

    using facet::obj;
    using facet::vt;
} /*namespace xo*/

/* end obj.hpp */
