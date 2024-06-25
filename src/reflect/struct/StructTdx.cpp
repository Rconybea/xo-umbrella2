/* @file StructTdx.cpp */

#include "struct/StructTdx.hpp"

namespace xo {
    using std::uint32_t;

    namespace reflect {
        std::unique_ptr<StructTdx>
        StructTdx::make(std::vector<StructMember> member_v,
                        bool have_to_self_tp,
                        std::function<TaggedPtr (void*)> to_self_tp)
        {
            return std::unique_ptr<StructTdx>(new StructTdx(std::move(member_v),
                                                            have_to_self_tp,
                                                            std::move(to_self_tp)));
        } /*make*/

        TaggedPtr
        StructTdx::child_tp(uint32_t i, void * object) const
        {
            if (i >= this->member_v_.size()) {
                /* TODO: raise exception here? */
                return TaggedPtr::universal_null();
            }

            StructMember const & member_info = this->member_v_[i];

            return member_info.get_member_tp(object);

        } /*get_child*/

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
    } /*namespace reflect*/
} /*namespace xo*/

/* end StructTdx.cpp */
