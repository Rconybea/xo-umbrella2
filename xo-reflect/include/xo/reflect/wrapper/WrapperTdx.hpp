/** @file WrapperTdx.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "xo/reflect/TypeDescrExtra.hpp"
#include "xo/reflect/struct/StructMember.hpp"
#include <memory>

namespace xo {
    namespace reflect {
        /** @brief extra type-associated information for a transparent
         *  wrapper: a type that stands for one value it holds, e.g. an id
         *  wrapping an integer (fn::CallbackId).
         *
         *  Its metatype is mt_atomic, with no children: a consumer treats it
         *  as a scalar.  What it adds is its value -- the wrapped member, in
         *  place (wrapped_tp()), so a consumer can show it as that value
         *  rather than as a struct holding it.
         *
         *  Reached through TypeDescrExtra::wrapper_info()
         *  (TypeDescr::is_wrapper()).  Installed by WrapperReflector
         *  (WrapperReflector.hpp).  See .xo-backlog/xo-reflect/issues/04.
         **/
        class WrapperTdx : public TypeDescrExtra {
        public:
            /** wrapping the member that @p accessor reaches **/
            static std::unique_ptr<WrapperTdx> make(std::unique_ptr<AbstractStructMemberAccessor> accessor);

            /** the wrapped member's type **/
            TypeDescr wrapped_td() const { return accessor_->member_td(); }
            /** the wrapped member of the wrapper at @p object, in place **/
            TaggedPtr wrapped_tp(void * object) const;

            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_atomic; }
            virtual uint32_t n_child(void * /*object*/) const override { return 0; }
            virtual uint32_t n_child_fixed() const override { return 0; }
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override;
            virtual const TypeDescrBase * fixed_child_td(uint32_t i) const override;
            virtual std::string const & struct_member_name(uint32_t i) const override;
            virtual const WrapperTdx * wrapper_info() const override { return this; }

        private:
            explicit WrapperTdx(std::unique_ptr<AbstractStructMemberAccessor> accessor)
                : accessor_{std::move(accessor)} {}

        private:
            /** reaches the wrapped member **/
            std::unique_ptr<AbstractStructMemberAccessor> accessor_;
        }; /*WrapperTdx*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end WrapperTdx.hpp */
