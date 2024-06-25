/* @file StructReflector.hpp */

#pragma once

#include "Reflect.hpp"
#include "TypeDescr.hpp"
#include "struct/StructMember.hpp"
#include "struct/StructTdx.hpp"
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

            template<typename OwnerT, typename MemberT>
            void reflect_member(std::string const & member_name,
                                MemberT OwnerT::* member_addr) {

                auto accessor
                    (GeneralStructMemberAccessor<StructT, OwnerT, MemberT>::make(member_addr));

                /* used to do this in GeneralStructMemberAccessor<> ctor,
                 * but that introduces #include cycle
                 */
                Reflect::require<MemberT>();

                this->member_v_.emplace_back(member_name, std::move(accessor));
            } /*reflect_member*/

            void require_complete() {
                if(!s_reflected_flag) {
                    s_reflected_flag = true;

                    constexpr bool have_to_self_tp = std::is_base_of_v<SelfTagging, StructT>;

                    /* if self-tagging,  can use .self_tp() to get most-derived tagged pointer */
                    auto to_self_tp_fn
                        = ([](void * object)
                            {
                                return SelfTagger<StructT, have_to_self_tp>::self_tp(object);
                            });

                    auto tdx = StructTdx::make(std::move(this->member_v_),
                                               have_to_self_tp,
                                               to_self_tp_fn);

                    this->td_->assign_tdextra(std::move(tdx));
                }
            } /*complete*/

            template<typename AncestorT>
            void adopt_ancestors() {
                assert(Reflect::is_reflected<AncestorT>());

                TypeDescr ancestor_td = Reflect::require<AncestorT>();

                /* requires that reflection of AncestorT has completed */
                {
                    assert(ancestor_td->is_struct());
                    assert(ancestor_td->complete_flag());
                }

                /* for structs,
                 * we know that object argument to TypeDescr::n_child() is unused
                 */
                for (uint32_t i = 0, n = ancestor_td->n_child(nullptr); i < n; ++i) {
                    StructMember const & member = ancestor_td->struct_member(i);

                    this->member_v_.push_back(member.for_descendant<StructT, AncestorT>());
                }
            } /*adopt_ancestors*/

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
