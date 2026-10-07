/** @file WrapperReflector.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "Reflect.hpp"
#include "TypeDescr.hpp"
#include "wrapper/WrapperTdx.hpp"
#include "struct/StructMember.hpp"
#include <cassert>
#include <memory>

namespace xo {
    namespace reflect {
        /** @brief describe @p WrapperT to xo-reflect as a transparent
         *  wrapper: a scalar that stands for one member it holds.
         *
         *  Use, for a type such as
         *
         *    class CallbackId { .. private: uint32_t id_; };
         *
         *  where reflecting it as a struct would make a consumer show a
         *  struct holding one number, rather than the number:
         *
         *    WrapperReflector<CallbackId> wr;
         *
         *    if (wr.is_incomplete())
         *        wr.reflect_wrapped(CallbackId::id_address());
         *    // completes when wr goes out of scope (or: wr.require_complete())
         *
         *  Then Reflect::require<CallbackId>() has metatype mt_atomic,
         *  is_wrapper() true, and wrapper_info()->wrapped_tp(obj) reaches the
         *  member in place.
         *
         *  See .xo-backlog/xo-reflect/issues/04.
         **/
        template <typename WrapperT>
        class WrapperReflector {
        public:
            using wrapper_t = WrapperT;

        public:
            WrapperReflector() : td_{EstablishTypeDescr::establish<WrapperT>()} {}
            ~WrapperReflector() {
                this->require_complete();
            }

            bool is_complete() const { return s_reflected_flag; }
            bool is_incomplete() const { return !s_reflected_flag; }
            TypeDescr td() const { return td_; }

            /** the member @p member_addr is the wrapped value; once **/
            template <typename OwnerT, typename MemberT>
            void reflect_wrapped(MemberT OwnerT::* member_addr) {
                assert(!this->accessor_ && "WrapperReflector: one wrapped member");

                /* as StructReflector::reflect_member */
                Reflect::require<MemberT>();

                this->accessor_
                    = GeneralStructMemberAccessor<WrapperT, OwnerT, MemberT>::make(member_addr);
            }

            /** install the wrapped member chosen so far; once per type **/
            void require_complete() {
                if (!s_reflected_flag && this->accessor_) {
                    s_reflected_flag = true;

                    static detail::InvokerAux<WrapperT> s_final_invoker;

                    this->td_->assign_tdextra(&s_final_invoker,
                                              WrapperTdx::make(std::move(this->accessor_)));
                }
            } /*require_complete*/

        private:
            /* set once WrapperT's WrapperTdx is installed */
            static bool s_reflected_flag;

            /* type description for WrapperT */
            TypeDescrW td_;
            /* reaches the wrapped member */
            std::unique_ptr<AbstractStructMemberAccessor> accessor_;
        }; /*WrapperReflector*/

        template <typename WrapperT>
        bool WrapperReflector<WrapperT>::s_reflected_flag = false;
    } /*namespace reflect*/
} /*namespace xo*/

/* end WrapperReflector.hpp */
