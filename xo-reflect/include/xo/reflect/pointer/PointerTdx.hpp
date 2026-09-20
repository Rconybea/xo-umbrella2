/* file PointerTdx.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "xo/reflect/EstablishTypeDescr.hpp"
#include "xo/reflect/TypeDescrExtra.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/tag.hpp>
#include <stdexcept>   /* std::runtime_error -- used to arrive via indentlog */
#include <type_traits>

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

            /* number of children unknown at compile time.
             * null-pointer -> 0, non-null pointer -> 1
             */
            virtual uint32_t n_child_fixed() const override { return 0; /*unknown*/ }
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override = 0;
            /* (forbidden) */
            virtual std::string const & struct_member_name(uint32_t i) const override;
        }; /*PointerTdx*/

        // ----- RefPointerTdx -----

        /* Pointer = xo::ref::intrusive_ptr<T> for some T */
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

            virtual TypeDescrBase * fixed_child_td(uint32_t /*i*/) const override {
                return EstablishTypeDescr::establish<typename Pointer::element_type>();
            }

            virtual TaggedPtr child_tp(uint32_t i, void * object) const override {
                using xo::pp::tostr;
                using xo::pp::xtag;

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

        // ----- RawPointerTdx -----

        /* Pointer = T* for some T -- a raw pointer.
         *
         * Same treatment as RefPointerTdx gives xo::ref::rp<T>: 0 children when
         * null, 1 otherwise.  The difference rp<T> carries and T* does not is a
         * LIFETIME guarantee; see .xo-backlog/xo-reflect/issues/01 for why that
         * is deliberately not reflection's problem -- traversal assumes valid
         * state exactly as every other operation does.
         *
         * WHETHER a raw pointer gets exposed at all is decided one level up, at
         * StructReflector::reflect_member: a member nobody names is not
         * described.  This class only says what happens to one somebody did.
         */
        template<typename T>
        class RawPointerTdx : public PointerTdx {
        public:
            /* the pointee AS REFLECTED, cv stripped.
             *
             * typeid ignores top-level cv, so require<const Foo>() and
             * require<Foo>() already resolve to one TypeDescr -- but the
             * canonical_name is taken from whichever call arrives first, and a
             * struct's json "_name_" comes from that name.  Stripping here
             * makes the name independent of establishment order.
             *
             * Note this does NOT merge the POINTER types: typeid(const Foo*)
             * and typeid(Foo*) differ, since there the const is not top-level.
             * So pointer-to-const and pointer-to-mutable keep their own
             * descriptors while sharing one pointee.
             */
            using pointee_t = std::remove_cv_t<T>;
            using target_t = T *;

            static std::unique_ptr<RawPointerTdx> make() {
                return std::unique_ptr<RawPointerTdx>(new RawPointerTdx());
            } /*make*/

            virtual uint32_t n_child(void * object) const override {
                if constexpr (std::is_void_v<T>) {
                    /* a void pointee has no representation to traverse into,
                     * so it reports no children even when non-null
                     */
                    return 0;
                } else {
                    target_t * ptr = reinterpret_cast<target_t *>(object);

                    return *ptr ? 1 : 0;
                }
            } /*n_child*/

            virtual TypeDescrBase * fixed_child_td(uint32_t /*i*/) const override {
                return EstablishTypeDescr::establish<pointee_t>();
            }

            virtual TaggedPtr child_tp(uint32_t i, void * object) const override {
                using xo::pp::tostr;
                using xo::pp::xtag;

                if constexpr (std::is_void_v<T>) {
                    /* unreachable: n_child() is always 0 above */
                    return TaggedPtr::universal_null();
                } else {
                    target_t * ptr = reinterpret_cast<target_t *>(object);

                    if (i > 0) {
                        throw std::runtime_error(tostr("RawPointerTdx<T>::child_tp"
                                                       ": attempt to fetch child #i from a T*",
                                                       xtag("T", type_name<target_t>()),
                                                       xtag("i", i),
                                                       xtag("n", this->n_child(object))));
                    }

                    /* const_cast because TaggedPtr is void*-based and cannot
                     * carry a pointer-to-const.  Reflection only reads through
                     * it; a caller that recovers the native type gets back the
                     * cv-unqualified pointee, which is the same compromise
                     * TaggedPtr makes everywhere else.
                     */
                    return establish_most_derived_tp(const_cast<pointee_t *>(*ptr));
                }
            } /*child_tp*/
        }; /*RawPointerTdx*/

    } /*namespace reflect*/
} /*namespace xo*/

/* end PointerTdx.hpp */
