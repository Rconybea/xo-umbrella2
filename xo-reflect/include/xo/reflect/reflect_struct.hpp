/** @file reflect_struct.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "StructReflector.hpp"
#include "xo/reflectutil/reflect_struct_info.hpp"

namespace xo {
    namespace reflect {
        namespace detail {
            /**
             *  @pre reflect_struct_member<T,MemberIx> will separately
             *  have been specialized for T.
             *  See discussion in [reflect_struct_info.hpp]
             *
             *
             **/
            template <typename T,
                      std::size_t MemberIx,
                      std::size_t N_Member>
            struct sr_member_helper {
                /** reflect members starting from member with index number @tparam MemberIx
                 *
                 *  @pre Members [0,..,MemberIx-1] must be already represented in @p *p_sr
                 **/
                static void add_members_from(StructReflector<T> * p_sr) {
                    const auto & member_info
                        = reflect_struct_member<T, MemberIx>().get();

                    p_sr->reflect_member(member_info.member_name_.c_str(),
                                         member_info.member_addr_);

                    /** reflect remaining members **/
                    sr_member_helper<T, MemberIx+1, N_Member>::add_members_from(p_sr);
                }
            };

            template <typename T, std::size_t MemberIx>
            struct sr_member_helper<T, MemberIx, MemberIx /*N_Member*/> {
                /** base case -- all members have been refleccted **/
                static void add_members_from(StructReflector<T> *) {}
            };
        } /*namespace detail*/


        /** It's awkward to have Reflect::reflect<>() do the right thing,
         *  because there's no way to specialize on whether a type T is a struct.
         *
         *  Use
         *    xo::reflect::Reflect::reflect_struct<T>() instead
         **/
        template <typename T>
        TypeDescr reflect_struct() {
            StructReflector<T> sr;

            if (sr.is_incomplete())
                detail::sr_member_helper<T, 0, reflect_struct_traits<T>::n_members>::add_members_from(&sr);

            /* TODO: handle composition: where T inherits another reflected type */
            /* TODO: handle multiple inheritance **/

            return sr.td();
        }
    } /*namespace reflect*/
} /*namespace xo*/


/** end reflect_struct.hpp **/
