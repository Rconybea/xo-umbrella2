/** @file box.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "obj.hpp"

namespace xo {
    namespace facet {

        /** object with owned state
         *  - with default DRepr argument:
         *    type-erased container (runtime polymorphism).
         *  - with sepcific DRepr argument:
         *    typed container (comptime polymorphism).
         **/
        template <typename AFacet, typename DRepr = DVariantPlaceholder>
        struct box : public RoutingType<AFacet, OObject<AFacet, DRepr>> {
            using Super = RoutingType<AFacet, OObject<AFacet, DRepr>>;

            box() : Super() {}

            /** box takes ownership of data @p *d;
             *  will destroy when box goes out of scope.
             *
             *  Note this is not useful when DRepr=DVariablePlaceholder
             **/
            explicit box(Super::DataPtr d) : Super(d) {}

            /** Adopt instance that has interface @p iface and (type-erased here)
             *  representation @p data
             **/
            box(const AFacet * iface, void * data)
            requires std::is_same_v<DRepr, DVariantPlaceholder>
            : Super(iface, data)
            {}

            /** (copy ctor not supported -- ownership is unique) **/
            box(const box & other) = delete;

            // --------------------------------

            /** Move constructor **/
            template <typename DOther>
            box(box<AFacet, DOther> && other)
            requires (std::is_same_v<DRepr, DVariantPlaceholder>
                      || std::is_same_v<DRepr, DOther>)
            : RoutingType<AFacet,OObject<AFacet,DRepr>>()
            {
                /* replacing .iface_ along w/ .data_ */
                this->from_obj(other);

                other.reset_opaque(nullptr);
            }

            /** explicit conversion to obj<AFacet,DRepr> **/
            obj<AFacet, DRepr> to_op() const noexcept {
                return obj<AFacet, DRepr>(this->iface(), this->data());
            }

            /** Take ownership from unowned object **/
            template <typename DOther>
            box & adopt(const obj<AFacet, DOther> & other)
            requires (std::is_same_v<DRepr, DVariantPlaceholder>
                      || std::is_same_v<DRepr, DOther>)
            {
                /* replace .iface_ along w/ .data_ */
                this->from_obj(other);

                return *this;
            }

            ~box() {
                auto p = this->data();
                if (p) {
                    this->_drop();
                    ::operator delete(p);
                }
            }
        };
    } /*namespace facet*/

    using facet::box;
} /*namespace xo*/

/* end box.hpp */
