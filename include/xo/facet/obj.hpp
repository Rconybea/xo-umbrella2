/** @file obj.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "RRouter.hpp"

namespace xo {
    namespace facet {
        /** object with borrowed state pointer
         *  - With default Data argument:
         *    type-erased polymorphic container
         *  - with specific Data argument:
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

            obj() {}
            explicit obj(Super::DataPtr d) : Super(d) {}

            /** copy constructor **/
            template <typename DOther>
            obj(const obj<AFacet, DOther> && other) : Super()
            {
                if constexpr (std::is_convertible_v<DRepr, DOther>) {
                    this->data_ = other.data_;
                } else if constexpr (std::is_same_v<DRepr, DVariantPlaceholder>) {
                    this->from_data(other.data_);
                } else {
                    /* still need something for downcasting */

                    static_assert(false, "expect DOther compatible with DRepr");
                }
            }

            /** move constructor from a different representation.
             *  allowed given:
             *  - same abstract interface
             *  - same strategy for holding state (naked / unique / refcounted ...)
             **/
            template <typename DOther>
            obj(const obj<AFacet, DOther> && other)
                    requires (std::is_same_v<DRepr, DVariantPlaceholder>
                              || std::is_convertible_v<DOther*, DRepr>)
              : Super()
            {
                static_assert(sizeof(obj<AFacet, DOther>)
                              == sizeof(obj<AFacet, DRepr>));

                other.move2any(this);

                assert(other.data_ = nullptr);
            }

            /** safe downcast from variant. null if downcast fails **/
            static obj from(const OObject<AFacet> & other) {
                return obj(other.template downcast<DRepr>());
            }
        };

        template <typename AFacet, typename DRepr>
        inline obj<AFacet, DRepr>
        with_facet(DRepr * data) {
            return obj<AFacet, DRepr>(data);
        }
    } /*namespace facet*/
} /*namespace xo*/

/* end obj.hpp */
