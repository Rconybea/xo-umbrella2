/** @file ReferenceTdx.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "PointerTdx.hpp"
#include "PointerRepr.hpp"
#include <xo/reflectutil/type_name.hpp>
#include <stdexcept>
#include <type_traits>

namespace xo {
    namespace reflect {
        // ----- ReferenceTdx -----

        /* Ref = T & or T && -- a reference.
         *
         * A pointer-like type (Metatype::mt_pointer) whose pointee is the
         * referent, cv-stripped and shared with T *'s (see
         * RawPointerTdx::pointee_t).  Edge ownership: borrowed, like T *.
         *
         * Described, not traversed: a reference is not an object, so nothing
         * builds a TaggedPtr OF reference type (yet).  Its use is in function
         * signatures -- std::function<R (const A &)>'s argument types.  So the
         * value operations throw.  Reference MEMBERS stay unreflectable: C++
         * has no member pointer for them.
         * See .xo-backlog/std-function-reflection/spec.md, section 1b
         */
        template<typename Ref>
        class ReferenceTdx : public PointerTdx {
        public:
            static_assert(std::is_reference_v<Ref>);

            using pointee_t = std::remove_cv_t<std::remove_reference_t<Ref>>;

            static std::unique_ptr<ReferenceTdx> make() {
                return std::unique_ptr<ReferenceTdx>(new ReferenceTdx());
            } /*make*/

            virtual PointerRepr pointer_repr() const override {
                return (std::is_lvalue_reference_v<Ref>
                        ? PointerRepr::lvalue_reference
                        : PointerRepr::rvalue_reference);
            }

            /* a reference does not own its referent */
            virtual Ownership child_edge_ownership() const override { return Ownership::borrowed; }

            virtual TypeDescrBase * fixed_child_td(uint32_t /*i*/) const override {
                return EstablishTypeDescr::establish<pointee_t>();
            }

            virtual void const * pointee_address(void * /*object*/) const override {
                this->not_traversed("pointee_address");
                return nullptr;
            }

            virtual uint32_t n_child(void * /*object*/) const override {
                this->not_traversed("n_child");
                return 0;
            }

            virtual TaggedPtr child_tp(uint32_t /*i*/, void * /*object*/) const override {
                this->not_traversed("child_tp");
                return TaggedPtr::universal_null();
            }

        private:
            ReferenceTdx() = default;

            [[noreturn]] void not_traversed(char const * op) const {
                using xo::pp::tostr;
                using xo::pp::xtag;

                throw std::runtime_error(tostr("ReferenceTdx::", op,
                                               ": a reference type is described, not traversed",
                                               xtag("T", type_name<Ref>())));
            }
        }; /*ReferenceTdx*/
    } /*namespace reflect*/
} /*namespace xo*/

/** end ReferenceTdx.hpp **/
