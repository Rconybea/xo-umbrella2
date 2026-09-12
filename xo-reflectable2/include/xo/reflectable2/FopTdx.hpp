/** @file FopTdx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "detail/AReflectable.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/pointer/PointerTdx.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <stdexcept>
#include <type_traits>

namespace xo {
    namespace reflect {
        /** @class FopTdx
         *  @brief Extra type-associated information for a faceted object pointer.
         *
         *  A fop is a facet pointer plus a data pointer, so it reflects
         *  as a pointer with one child -- its representation.  That is the same
         *  shape @c RefPointerTdx gives @c xo::ref::rp<T>, and it means
         *  xo-printjson needs no new case: @c print_generic_pointer already
         *  walks @c n_child() / @c get_child(0).
         *
         *  Two cases, differing in whether the representation is known at
         *  compile time:
         *  - @c DRepr concrete: the child's TypeDescr is @c Reflect::require<DRepr>().
         *  - @c DRepr = @c DVariantPlaceholder (i.e. @c vt<AFacet>): erased.
         *    Recovered at runtime by rotating through @c FacetRegistry to
         *    @c AReflectable and asking it for @c self_tp().  A representation
         *    that has NOT opted in throws -- that is a programming error, not
         *    a data-dependent condition, so it must not render as empty.
         *
         *  Note which case is routine: D-types hold erased fop members as a
         *  matter of course (@c DDictionary, @c DList, @c DArray), so the
         *  concrete case is the exception rather than the rule.
         **/
        template <typename AFacet, typename DRepr>
        class FopTdx : public PointerTdx {
        public:
            using target_t = xo::facet::obj<AFacet, DRepr>;

            /** true iff the representation is type-erased **/
            static constexpr bool c_erased
                = std::is_same_v<DRepr, xo::facet::DVariantPlaceholder>;

            static std::unique_ptr<FopTdx> make() {
                return std::unique_ptr<FopTdx>(new FopTdx());
            }

            // ----- Inherited from PointerTdx -----

            virtual uint32_t n_child(void * object) const override {
                /* same in both cases: a fop has a child exactly when its data
                 * pointer is non-null.  Counting needs no rotation.
                 */
                target_t * p = reinterpret_cast<target_t *>(object);

                return (p->data() ? 1 : 0);
            } /*n_child*/

            /** Resolve an ERASED fop to its representation.
             *
             *  @c PrintJson::print_tp does NOT call @c most_derived_self_tp --
             *  it goes straight to the metatype switch -- so the pointer path
             *  below must rotate as well, or a directly-handed erased fop
             *  prints as {} while a nested one prints correctly.
             **/
            virtual TaggedPtr most_derived_self_tp(TypeDescrBase const * object_td,
                                                   void * object) const override {
                if constexpr (c_erased) {
                    target_t * p = reinterpret_cast<target_t *>(object);

                    if (p->data())
                        return repr_tp(p);
                }

                return TypeDescrExtra::most_derived_self_tp(object_td, object);
            } /*most_derived_self_tp*/

            virtual const TypeDescrBase * fixed_child_td(uint32_t /*i*/) const override {
                if constexpr (c_erased) {
                    return nullptr;
                } else {
                    return EstablishTypeDescr::establish<DRepr>();
                }
            } /*fixed_child_td*/

            virtual TaggedPtr child_tp(uint32_t i, void * object) const override {
                using xo::pp::tostr;
                using xo::pp::xtag;

                if (i >= this->n_child(object)) {
                    throw std::runtime_error
                        (tostr("FopTdx<AFacet,DRepr>::child_tp"
                               ": attempt to fetch child #i from a faceted object pointer",
                               xtag("T", type_name<target_t>()),
                               xtag("i", i),
                               xtag("n", this->n_child(object))));
                }

                target_t * p = reinterpret_cast<target_t *>(object);

                if constexpr (c_erased) {
                    return repr_tp(p);
                } else {
                    return establish_most_derived_tp(p->data());
                }
            } /*child_tp*/

        private:
            /** Rotate an erased fop to @c AReflectable and take its
             *  @c self_tp().  Only instantiated for the erased case.
             *
             *  Uses the THROWING @c variant(), not @c try_variant(): a
             *  representation that has not opted in is a programming error.
             *  The thrown message names the representation, so a reader is not
             *  handed a bare typeseq number to go look up.
             **/
            static TaggedPtr repr_tp(target_t * p) {
                auto reflectable
                    = xo::facet::FacetRegistry::instance()
                          .template variant<AReflectable>(*p);

                return reflectable.self_tp();
            } /*repr_tp*/
        }; /*FopTdx*/

        // ----- xo::facet::obj<AFacet, DRepr> -----

        template <typename AFacet, typename DRepr>
        class EstablishTdx<xo::facet::obj<AFacet, DRepr>> {
        public:
            static std::unique_ptr<TypeDescrExtra> make() {
                if constexpr (!FopTdx<AFacet, DRepr>::c_erased) {
                    /* ensure the representation is reflected before anything
                     * asks this object for its child
                     */
                    Reflect::require<DRepr>();
                }

                return FopTdx<AFacet, DRepr>::make();
            } /*make*/
        }; /*EstablishTdx<obj<AFacet,DRepr>>*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end FopTdx.hpp */
