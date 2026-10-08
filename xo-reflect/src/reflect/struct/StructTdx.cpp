/* @file StructTdx.cpp */

#include "struct/StructTdx.hpp"
#include "TypeDescr.hpp"

namespace xo {
    using std::uint32_t;

    namespace reflect {
        std::unique_ptr<StructTdx>
        StructTdx::make(std::vector<StructMember> member_v,
                        GuardVector guard_v,
                        bool have_to_self_tp,
                        std::function<TaggedPtr (void*)> to_self_tp)
        {
            return std::unique_ptr<StructTdx>(new StructTdx(std::move(member_v),
                                                            std::move(guard_v),
                                                            have_to_self_tp,
                                                            std::move(to_self_tp)));
        } /*make*/

        StructTdx::StructTdx(std::vector<StructMember> member_v,
                             GuardVector guard_v,
                             bool have_to_self_tp,
                             std::function<TaggedPtr (void*)> to_self_tp)
            : member_v_{std::move(member_v)},
              guard_v_{std::move(guard_v)},
              guard_members_v_(guard_v_.size()),
              have_to_self_tp_{have_to_self_tp},
              to_self_tp_{std::move(to_self_tp)}
        {
            /* group members by guard, once, so a traversal need not scan */
            for (uint32_t i = 0, n = member_v_.size(); i < n; ++i) {
                std::optional<uint32_t> g = member_v_[i].guard_ix();

                if (g) {
                    assert(*g < guard_v_.size());

                    guard_members_v_.at(*g).push_back(i);
                } else {
                    unguarded_v_.push_back(i);
                }
            }
        } /*ctor*/

        TaggedPtr
        StructTdx::child_tp(uint32_t i, void * object) const
        {
            if (i >= this->member_v_.size()) {
                /* TODO: raise exception here? */
                return TaggedPtr::universal_null();
            }

            const StructMember & member_info = this->member_v_[i];

            return member_info.get_member_tp(object);

        } /*get_child*/

        TypeDescr
        StructTdx::fixed_child_td(uint32_t i ) const
        {
            if (i >= this->member_v_.size())
                return nullptr;

            const StructMember & member_info = this->member_v_[i];

            return member_info.get_member_td();
        } /*fixed_child_td*/

        std::string const &
        StructTdx::struct_member_name(uint32_t i) const
        {
            StructMember const * sm = this->struct_member(i);

            return sm->member_name();
        } /*struct_member_name*/

        StructMember const *
        StructTdx::struct_member(uint32_t i) const
        {
            if (i >= this->member_v_.size()) {
                /* TODO: raise exception here */
                assert(false);
                return nullptr;
            }

            return &(this->member_v_[i]);
        } /*struct_member*/

        TaggedPtr
        StructTdx::guard_tp(uint32_t g, void * object) const
        {
            return this->guard(g).member_tp(object);
        } /*guard_tp*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end StructTdx.cpp */
