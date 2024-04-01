/* @file native_bpu_concept.hpp */

#pragma once

#include "ratio_concept.hpp"
#include "dim_util.hpp"
#include <concepts>

namespace xo {
    namespace obs {
        /**
         *  e.g. see native_bpu<native_dim_id, std::ratio<..>>
         *
         *  bpu short for 'basis power unit'.
         *
         *  NOTE: in typical c++ use,  there won't be a reason to declare
         *        a variable of type NativeBpu.  Instead will appear
         *        as a template argument.
         **/
        template <typename NativeBpu>
        concept native_bpu_concept = requires(NativeBpu bpu)
        {
            typename NativeBpu::scalefactor_type;
            typename NativeBpu::power_type;

            // NativeBpu::c_native_dim :: native_dim_id
            // NativeBpu::c_scale :: std::intmax_t
            // NativeBpu::num :: int
            // NativeBpu::den :: int
        }
            && ((std::is_same_v<decltype(NativeBpu::c_native_dim), const dim>)
                && ratio_concept<typename NativeBpu::scalefactor_type>
                && ratio_concept<typename NativeBpu::power_type>
                && (std::is_signed_v<decltype(NativeBpu::c_num)>)
                && (std::is_signed_v<decltype(NativeBpu::c_den)>))
            // && std::copyable<Foo>
            ;
    } /*namespace obs*/
} /*namespace xo*/


/* end native_bpu_concept.hpp */
