/* @file StructReflector.hpp */

#pragma once

#include "Reflect.hpp"
#include "SelfTaggingDisplayable.hpp"
#include "TypeDescr.hpp"
#include "struct/StructMember.hpp"
#include "struct/StructTdx.hpp"
#include <type_traits>
#include <vector>

namespace xo {
    namespace reflect {
        template<typename StructT, bool IsSelfTaggingDescendant>
        class SelfTagger {};

        template<typename StructT>
        struct SelfTagger<StructT, true> {
            static TaggedPtr self_tp(void * object) {
                return (reinterpret_cast<StructT *>(object))->self_tp();
            }
        };

        template<typename StructT>
        struct SelfTagger<StructT, false> {
            static TaggedPtr self_tp(void * /*object*/) { assert(false); return TaggedPtr::universal_null(); }
        };

        /** @brief one member just declared to a StructReflector<StructT>:
         *  qualifies it, e.g.
         *
         *    REFLECT_MEMBER(sr, sender).owning();
         *    REFLECT_MEMBER(sr, pjson).borrowed();
         *    REFLECT_MEMBER(sr, outbound_q).guarded_by(&Recd::mutex_);
         *
         *  Valid while its StructReflector is: it holds the reflector and
         *  the member's position.
         **/
        template <typename StructT>
        class StructMemberDecl {
        public:
            StructMemberDecl(StructReflector<StructT> * reflector, uint32_t member_ix)
                : reflector_{reflector}, member_ix_{member_ix} {}

            /** override the member type's default ownership (Ownership.hpp).
             *  Only a pointer member: an object held by value has no other
             *  home
             **/
            StructMemberDecl & ownership(Ownership x) {
                StructMember & m = this->member();

                assert(m.get_member_td()->is_pointer()
                       && "StructMemberDecl: ownership override on a non-pointer member");

                m.ownership_ = x;
                return *this;
            }
            StructMemberDecl & owning() { return this->ownership(Ownership::owning); }
            StructMemberDecl & shared() { return this->ownership(Ownership::shared); }
            StructMemberDecl & borrowed() { return this->ownership(Ownership::borrowed); }

            /** this member is read only with lockable @p guard held: a
             *  member of StructT (or of a base), not necessarily reflected.
             *  Members guarded by the same lockable share one guard.  Once
             *  per member.  See .xo-backlog/xo-reflect/issues/08
             **/
            template <typename MutexT, typename OwnerT>
            StructMemberDecl & guarded_by(MutexT OwnerT::* guard) {
                static_assert(std::is_base_of_v<OwnerT, StructT>,
                              "StructMemberDecl::guarded_by: guard must be a member of StructT or a base");

                StructMember & m = this->member();

                assert(!m.guard_ix_ && "StructMemberDecl: member already guarded");

                [[maybe_unused]] TypeDescr guard_td = Reflect::require<MutexT>();

                assert(guard_td->is_lockable()
                       && "StructMemberDecl::guarded_by: guard type is not a reflected lockable");

                m.guard_ix_ = reflector_->intern_guard
                                  (GeneralStructMemberAccessor<StructT, OwnerT, MutexT>::make(guard));
                return *this;
            }

        private:
            StructMember & member() { return reflector_->member_v_.at(member_ix_); }

        private:
            /* reflector this member was declared to */
            StructReflector<StructT> * reflector_ = nullptr;
            /* the member's position in .reflector's member list */
            uint32_t member_ix_ = 0;
        }; /*StructMemberDecl*/

        /* RAII pattern for reflecting a struct.
         *
         * Use:
         *   struct Foo { int x_; double y_; };
         *
         *   StructReflector<Foo> sr;
         *   REFLECT_LITERAL_MEMBER(sr, x_);
         *   REFLECT_LITERAL_MEMBER(sr, y_);
         *
         *   // optional: regardless, reflection will be completed when sr goes out of scope
         *   sr.require_complete();
         */
        template<typename StructT>
        class StructReflector {
        public:
            using struct_t = StructT;

        public:
            StructReflector() : td_{EstablishTypeDescr::establish<StructT>()} {}
            ~StructReflector() {
                this->require_complete();
            }

            bool is_complete() const { return s_reflected_flag; }
            bool is_incomplete() const { return !s_reflected_flag; }
            TypeDescr td() const { return td_; }

            /** declare member @p member_name; qualify it through the
             *  returned StructMemberDecl, or ignore it
             **/
            template<typename OwnerT, typename MemberT>
            StructMemberDecl<StructT> reflect_member(std::string const & member_name,
                                                     MemberT OwnerT::* member_addr) {
                /* used to do this in GeneralStructMemberAccessor<> ctor,
                 * but that introduces #include cycle.  Before StructMember's
                 * ctor, which reads the member type's default ownership
                 */
                Reflect::require<MemberT>();

                auto accessor
                    (GeneralStructMemberAccessor<StructT, OwnerT, MemberT>::make(member_addr));

                this->member_v_.emplace_back(member_name, std::move(accessor));

                return StructMemberDecl<StructT>(this, this->member_v_.size() - 1);
            } /*reflect_member*/

            void require_complete() {
                if(!s_reflected_flag) {
                    s_reflected_flag = true;

                    /* Base types providing virtual self_tp() -> TaggedPtr */
                    constexpr bool have_to_self_tp
                        = (std::is_base_of_v<SelfTagging, StructT>
                           || std::is_base_of_v<SelfTaggingDisplayable, StructT>);

                    /* if self-tagging,  can use .self_tp() to get most-derived tagged pointer */
                    auto to_self_tp_fn
                        = ([](void * object)
                            {
                                return SelfTagger<StructT, have_to_self_tp>::self_tp(object);
                            });

                    static detail::InvokerAux<StructT> s_final_invoker;

                    auto tdx = StructTdx::make(std::move(this->member_v_),
                                               std::move(this->guard_v_),
                                               std::move(this->parent_v_),
                                               have_to_self_tp,
                                               to_self_tp_fn);

                    this->td_->assign_tdextra(&s_final_invoker,
                                              std::move(tdx));
                }
            } /*complete*/

            /** declare @p AncestorT a parent of StructT: record it
             *  (TypeDescr::is_derived_from), and adopt its reflected members
             *  and their guards.  Once per base, for multiple inheritance.
             *
             *  require: AncestorT reflected, complete.  C++ cannot tell a
             *  direct base from a more distant one; either serves, since
             *  derivation is transitive
             **/
            template<typename AncestorT>
            void adopt_parent() {
                static_assert(std::is_base_of_v<AncestorT, StructT>,
                              "StructReflector::adopt_parent: AncestorT must be a base of StructT");

                assert(Reflect::is_reflected<AncestorT>());

                TypeDescr ancestor_td = Reflect::require<AncestorT>();

                /* requires that reflection of AncestorT has completed */
                {
                    assert(ancestor_td->is_struct());
                    assert(ancestor_td->complete_flag());
                }

                this->parent_v_.push_back(ancestor_td);

                /* the ancestor's guards, reached from StructT: appended, so an
                 * adopted member's guard index shifts by .guard_v's size before.
                 *
                 * Not interned against guards StructT declares itself: a
                 * mutex both declare would be two guards, taken in turn
                 * (never nested) by a traversal, splitting what could be one
                 * snapshot.
                 */
                StructTdx const * ancestor_tdx = static_cast<StructTdx const *>(ancestor_td->tdextra());
                uint32_t guard_offset = this->guard_v_.size();

                for (uint32_t g = 0, n = ancestor_tdx->n_guard(); g < n; ++g) {
                    this->guard_v_.push_back(AncestorStructMemberAccessor<StructT, AncestorT>::adopt
                                             (ancestor_tdx->guard(g).clone()));
                }

                /* for structs,
                 * we know that object argument to TypeDescr::n_child() is unused
                 */
                for (uint32_t i = 0, n = ancestor_td->n_child(nullptr); i < n; ++i) {
                    StructMember const & member = ancestor_td->struct_member(i);

                    StructMember adopted = member.for_descendant<StructT, AncestorT>();

                    if (adopted.guard_ix_)
                        adopted.guard_ix_ = *adopted.guard_ix_ + guard_offset;

                    this->member_v_.push_back(std::move(adopted));
                }
            } /*adopt_parent*/

        private:
            friend class StructMemberDecl<StructT>;

            /* index of @p guard in .guard_v, appending it if new */
            uint32_t intern_guard(std::unique_ptr<AbstractStructMemberAccessor> guard) {
                for (uint32_t g = 0, n = this->guard_v_.size(); g < n; ++g) {
                    if (this->guard_v_[g]->same_member(*guard))
                        return g;
                }

                this->guard_v_.push_back(std::move(guard));

                return this->guard_v_.size() - 1;
            } /*intern_guard*/

        private:
            /* set irrevocably to true when .complete() runs.
             *
             * want to reflect a particular type once;
             * short-circuit 2nd or later attempts on the same type
             */
            static bool s_reflected_flag;

            /* type description object for StructT */
            TypeDescrW td_;

            /* members of StructT (at least those we're choosing to reflect) */
            std::vector<StructMember> member_v_;
            /* lockables guarding members of StructT; StructMember::guard_ix() indexes it */
            StructTdx::GuardVector guard_v_;
            /* parents of StructT declared so far (adopt_parent) */
            std::vector<TypeDescr> parent_v_;
        }; /*StructReflector*/

        template<typename StructT>
        bool StructReflector<StructT>::s_reflected_flag = false;
    } /*namespace reflect*/

    /* e.g.
     *   struct Foo { int bar_; };
     *   struct Bar : public Foo { .. };
     *
     *   StructReflector<Bar> sr;
     *   REFLECT_EXPLICIT_MEMBER(sr, "bar", &Foo::bar_);
     */
#define REFLECT_EXPLICIT_MEMBER(sr, member_name, member) sr.reflect_member(member_name, member)

    /* e.g.
     *   struct Foo { int bar_; };
     *
     *   StructReflector<Foo> sr;
     *   REFLECT_LITERAL_MEMBER(sr, bar_);
     *
     * then REFLECT_LITERAL_MEMBER() expands to something like:
     *   sr.reflect_member("bar_", &StructReflector<Foo>::struct_t::bar_)
     */
#define REFLECT_LITERAL_MEMBER(sr, member_name) sr.reflect_member(#member_name, &decltype(sr)::struct_t::member_name)

    /* like REFLECT_LITERAL_MEMBER(),  but append trailing underscore
     *
     * minor convenience,  so we can write
     *   struct Foo { int bar_; };
     *
     *   StructReflector<Foo> sr;
     *   REFLECT_MEMBER(sr, bar);   // reflects Foo::bar_ as "bar"
     */
#define REFLECT_MEMBER(sr, member_name) sr.reflect_member(#member_name, &decltype(sr)::struct_t::member_name##_)

} /*namespace xo*/
