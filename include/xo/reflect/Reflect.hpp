/* file Reflect.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "SelfTagging.hpp"
#include "EstablishTypeDescr.hpp"
#include "atomic/AtomicTdx.hpp"
#include "pointer/PointerTdx.hpp"
#include "vector/VectorTdx.hpp"
#include "struct/StructTdx.hpp"
#include "xo/refcnt/Refcounted.hpp"
#include <vector>
#include <array>
#include <utility> // for std::pair<>

namespace xo {
    namespace reflect {
        template<typename T>
        class EstablishTdx {
        public:
            static std::unique_ptr<TypeDescrExtra> make() { return AtomicTdx::make(); }
        }; /*EstablishTdx*/

        // ----- xo::ref::rp<Object> -----

        /* definition provide after decl for Reflect {} below */
        template<typename Object>
        class EstablishTdx<xo::ref::rp<Object>> {
        public:
            static std::unique_ptr<TypeDescrExtra> make();
        }; /*EstablishTdx*/

        // ----- std::array<Element, N> -----

        /* definition provide after decl for Reflect {} below */
        template<typename Element, std::size_t N>
        class EstablishTdx<std::array<Element, N>> {
        public:
            static std::unique_ptr<TypeDescrExtra> make();
        }; /*EstablishTdx*/

        // ----- std::vector<Element> -----

        /* definition provide after decl for Reflect {} below */
        template<typename Element>
        class EstablishTdx<std::vector<Element>> {
        public:
            static std::unique_ptr<TypeDescrExtra> make();
        }; /*EstablishTdx*/

        // ----- std::pair<Lhs, Rhs> -----

        /* definition provide after decl for Reflect {} below */
        template<typename Lhs, typename Rhs>
        class EstablishTdx<std::pair<Lhs, Rhs>> {
        public:
            static std::unique_ptr<TypeDescrExtra> make();
        }; /*EstablishTdx*/

        // ----- MakeTagged -----

        template<typename T>
        class TaggedPtrMaker {
        public:
            static TaggedPtr make_tp(T * x);
            static TaggedRcptr make_rctp(T * x);
        };

        template<>
        class TaggedPtrMaker<SelfTagging> {
        public:
            static TaggedPtr make_tp(SelfTagging * x) {
                return x->self_tp();
            } /*make_tp*/

            static TaggedRcptr make_rctp(SelfTagging * x) {
                return x->self_tp();
            } /*make_rctp*/
        }; /*TaggedPtrMaker*/

        // ----- Reflect -----

        class Reflect {
        public:
            /* Use:
             *   using mytype = ...;
             *   if (Reflect::is_reflected<mytype>()) { ... }
             */
            template<typename T>
            static bool is_reflected() { return TypeDescrBase::is_reflected(&typeid(T)); }

            /* Use:
             *   using mytype = ...;
             *   TypeDescrW td = Reflect::require<mytype>();
             *
             * Note:
             * To avoid cyclic header dependencies
             * (between EstablishTypeDescr.hpp <-> {vector/VectorTdx.hpp etc.},
             * we use a 2-stage setup process:
             *
             * 1. EstablishTypeDescr::establish<T>() creates a TypeDescr* object
             *    with lowest-common-denominator .tdextra AtomicTdx.
             *    (see [reflect/EstablishTypeDescr.hpp])
             *
             * 2. Reflect::require<T>() upgrades .tdextra to suitable implementation
             *    depending on T;   this means also need to visit reflection info
             *    (TypeDescr objects) for nested types to upgrade them too.
             *
             * This allows template-fu for a compound type (like std::vector<T>),
             * implemented in specialized header (like [reflect/struct/VectorTdx.hpp]) to
             * refer to reflection info for T without having to pull in all the
             * headers needed to properly reflect T (like this [reflect/Reflect.hpp])
             *
             */
            template<typename T>
            static TypeDescrW require() {
                TypeDescrW retval_td = EstablishTypeDescr::establish<T>();

                /* mark TypeDescr for T as complete (even though it isn't quite yet),
                 * so that when we encounter recursive types,  reflection terminates.
                 * For example consider type resulting from code like
                 *
                 *    typename T;
                 *    using T = std::vector<T *>;
                 *
                 */
                if (retval_td->mark_complete()) {
                    /* control here on 2nd+later calls to require<T>().
                     * in principle can immediately short-circuit.
                     */
                } else {
                    /* control comes here the first time require<T>() runs */

                    auto final_tdx = EstablishTdx<T>::make();

                    retval_td->assign_tdextra(std::move(final_tdx));

                    /* also need to require for each child */
                }

                return retval_td;
            } /*require*/

            /* Use:
             *    T * xyz = ...;
             *    TaggedPtr xyz_tp = Reflect::make_tp(xyz);
             */
            template<typename T>
            static TaggedPtr make_tp(T * x) { return TaggedPtrMaker<T>::make_tp(x); }

            template<typename T>
            static TaggedRcptr make_rctp(T * x) { return TaggedPtrMaker<T>::make_rctp(x); }
        }; /*Reflect*/

        // ----- MakeTagged -----

        template<typename T>
        TaggedPtr
        TaggedPtrMaker<T>::make_tp(T * x) {
            return TaggedPtr(Reflect::require<T>(), x);
        } /*make_tp*/

        template<typename T>
        TaggedRcptr
        TaggedPtrMaker<T>::make_rctp(T * x) {
            return TaggedRcptr(Reflect::require<T>(), x);
        } /*make_rctp*/

        // ----- xo::ref::rp<Object> -----

        /* declared above before
         *   class Reflect { .. }
         */
        template<typename Object>
        std::unique_ptr<TypeDescrExtra>
        EstablishTdx<xo::ref::rp<Object>>::make() {
            /* need to ensure Object is property reflected.
             *
             * In practice must be a class type,  since has to store refcount
             * + supply assoc'd incr/decr methods
             */
            Reflect::require<Object>();

            return RefPointerTdx<xo::ref::rp<Object>>::make();
        } /*make*/

        // ----- std::array<Element, N> -----

        /* declared above before
         *   class Reflect { .. }
         */
        template<typename Element, std::size_t N>
        std::unique_ptr<TypeDescrExtra>
        EstablishTdx<std::array<Element, N>>::make() {
            /* need to ensure Element is properly reflected */
            Reflect::require<Element>();

            return StdArrayTdx<Element, N>::make();
        } /*make*/

        // ----- std::vector<Element> -----

        /* declared above before
         *   class Reflect { .. }
         */
        template<typename Element>
        std::unique_ptr<TypeDescrExtra>
        EstablishTdx<std::vector<Element>>::make() {
            /* need to ensure Element is properly reflected */
            Reflect::require<Element>();

            return StdVectorTdx<Element>::make();
        } /*make*/

        // ----- std::pair<Lhs, Rhs> -----

        /* declared above before
         *   class Reflect { .. }
         */
        template<typename Lhs, typename Rhs>
        std::unique_ptr<TypeDescrExtra>
        EstablishTdx<std::pair<Lhs, Rhs>>::make() {
            /* need to ensure Lhs, Rhs are properly reflected */
            Reflect::require<Lhs>();
            Reflect::require<Rhs>();

            return StructTdx::pair<Lhs, Rhs>();
        } /*make*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end Reflect.hpp */
