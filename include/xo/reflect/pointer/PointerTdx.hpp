/* file PointerTdx.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "xo/reflect/TypeDescrExtra.hpp"
#include "xo/reflect/EstablishTypeDescr.hpp"
#include "xo/indentlog/scope.hpp"

namespace xo {
    namespace reflect {
        /* Extra type-associated information for a pointer-like type
         *
         * Treat a pointer as a container that has 0 or 1 children;
         * - 0 children if null
         * - 1 child otherwise
         */
        class PointerTdx : public TypeDescrExtra {
        public:
            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_pointer; }
            virtual uint32_t n_child(void * object) const override = 0;
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override = 0;
            /* (forbidden) */
            virtual std::string const & struct_member_name(uint32_t i) const override;
        }; /*PointerTdx*/

        // ----- RefPointerTdx -----

        /* xo::ref::intrusive_ptr<T> for some T */
        template<typename Pointer>
        class RefPointerTdx : public PointerTdx {
        public:
            using target_t = Pointer;

            static std::unique_ptr<RefPointerTdx> make() {
                return std::unique_ptr<RefPointerTdx>(new RefPointerTdx());
            } /*make*/

            virtual uint32_t n_child(void * object) const override {
                /* e.g:
                 *   target_t = ref::rp<filter::KalmanFilterState>
                 */
                target_t * ptr = reinterpret_cast<target_t *>(object);

                if (*ptr)
                    return 1;
                else
                    return 0;
            } /*n_child*/

            virtual TaggedPtr child_tp(uint32_t i, void * object) const override {
                using xo::tostr;
                using xo::xtag;

                target_t * ptr = reinterpret_cast<target_t *>(object);

                if (i > 0) {
                    throw std::runtime_error(tostr("RefPointerTdx<T>::child_tp"
                                                   ": attempt to fetch child #i from a ref::rp<T>",
                                                   xtag("T", type_name<target_t>()),
                                                   xtag("i", i),
                                                   xtag("n", this->n_child(object))));
                }

                return establish_most_derived_tp(ptr->get());
            } /*child_tp*/
        }; /*RefPointerTdx*/

    } /*namespace reflect*/
} /*namespace xo*/

/* end PointerTdx.hpp */
