/** @file abox.hpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include "Allocator.hpp"
#include <xo/facet/obj.hpp>
#include <cassert>

namespace xo {
    namespace mm {

        /** object with owned state
         *  - with default DRepr argument:
         *    type-erased container (runtime polymorphism).
         *  - with sepcific DRepr argument:
         *    typed container (comptime polymorphism).
         *
         *  Similar to box<AFacet,DRepr> (see box.hpp in xo-facet/):
         *  1. inherits fat object pointer with AFacet*, DRepr* pair
         *  2. automatically calls polymorphic DRepr::~DRepr when
         *     abox goes out of scope.
         *  Unlike box<AFacet,DRepr>:
         *  3. gets memory from explicit arena-like allocator
         *  4. calls dtor DRepr::~DRepr(), but not delete.
         *     Does not retain allocator.
         **/
        template <typename AFacet, typename DRepr = xo::facet::DVariantPlaceholder>
        struct abox : public xo::facet::RoutingType<AFacet, xo::facet::OObject<AFacet, DRepr>> {
            using DVariantPlaceholder = xo::facet::DVariantPlaceholder;
            using Super = xo::facet::RoutingType<AFacet, xo::facet::OObject<AFacet, DRepr>>;

            abox() : Super() {}

            /** abox takes ownership of data @p *d;
             *  will destroy when abox goes out of scope.
             *
             *  Note this is not useful when DRepr=DVariablePlaceholder
             **/
            explicit abox(Super::DataPtr d) : Super(d) {}

            /** Adopt instance that has interface @p iface and (type-erased here)
             *  representation @p data
             **/
            abox(const AFacet * iface, void * data)
            requires std::is_same_v<DRepr, DVariantPlaceholder>
            : Super(iface, data)
            {}

            /** (copy ctor not supported -- ownership is unique) **/
            abox(const abox & other) = delete;

            /** Move constructor **/
            template <typename DOther>
            abox(abox<AFacet, DOther> && other)
            requires (std::is_same_v<DRepr, DVariantPlaceholder>
                      || std::is_same_v<DRepr, DOther>)
            : xo::facet::RoutingType<AFacet,xo::facet::OObject<AFacet,DRepr>>()
            {
                /* replacing .iface_ along w/ .data_ */
                this->from_obj(other);

                other.reset_opaque(nullptr);
            }

            /** allocates for sizeof(DRepr), so DRepr must not use flexible array **/
            template <typename... Args>
            static abox make(obj<AAllocator> alloc, Args&&... args) {
                void * mem = alloc.alloc_for<DRepr>();
                if (mem) {
                    DRepr * data = ::new (mem) DRepr(std::forward<Args>(args)...);

                    assert(data);

                    return abox(data);
                } else {
                    assert(false);

                    return abox();
                }
            }

            // --------------------------------

            /** explicit conversion to obj<AFacet,DRepr> **/
            obj<AFacet, DRepr> to_op() const noexcept {
                return obj<AFacet, DRepr>(this->iface(), this->data());
            }

            /** Take ownership from unowned object **/
            template <typename DOther>
            abox & adopt(const obj<AFacet, DOther> & other)
            requires (std::is_same_v<DRepr, DVariantPlaceholder>
                      || std::is_same_v<DRepr, DOther>)
            {
                /* replace .iface_ along w/ .data_ */
                this->from_obj(other);

                return *this;
            }

            ~abox() {
                auto p = this->data();
                if (p) {
                    this->_drop();
                }
            }
        };
    } /*namespace mm*/

    using mm::abox;
} /*namespace xo*/

/* end abox.hpp */
