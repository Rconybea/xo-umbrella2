/** @file FopTdx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/pointer/PointerTdx.hpp>
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
         *    Reports ZERO children for now, rather than guessing.  Recovering
         *    the representation needs a rotation through @c FacetRegistry to an
         *    @c AReflectable facet; see .xo-backlog/reflectable2/issues/03.
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
                if constexpr (c_erased) {
                    /* see issues/03: rotating to AReflectable is what makes
                     * the representation reachable.  Until then an erased
                     * object prints as {} rather than as a wrong answer.
                     */
                    return 0;
                } else {
                    target_t * p = reinterpret_cast<target_t *>(object);

                    return (p->data() ? 1 : 0);
                }
            } /*n_child*/

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

                if constexpr (c_erased) {
                    /* unreachable: n_child() is 0, so the guard above threw */
                    return TaggedPtr::universal_null();
                } else {
                    target_t * p = reinterpret_cast<target_t *>(object);

                    return establish_most_derived_tp(p->data());
                }
            } /*child_tp*/
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
