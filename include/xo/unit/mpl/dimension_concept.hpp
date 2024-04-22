/* @file dimension_concept.hpp */

#pragma once

#include "native_bpu_concept.hpp"

namespace xo {
    namespace unit {
        /** checks most non-empty BPU (basis power unit) node types;
         *  cannot check BpuList::rest_type,  because concept definition
         *  can't (as of c++23) be recursive.
         *
         *  As workaround,  revert to type traits,  seend below.
         **/
        template <typename BpuList>
        concept bpu_node_concept = requires(BpuList bpulist)
        {
            typename BpuList::front_type;
            typename BpuList::rest_type;
        }
            && (native_bpu_concept<typename BpuList::front_type>
                && (std::is_integral_v<decltype(BpuList::n_dimension)>)
                //&& (std::same_as<typename BpuList::front_type, void>
                //    or nonempty_bpu_list_concept<BpuList::rest_type>))
                );

        namespace detail {
            // ------------------------------------------------------------

            /* check for 'list of native_bpu_concept'.
             * Need type trait for this,  to access partial specializations
             */
            template < typename BpuList >
            struct bpu_list_traits;

            /* void (representing empty list) is fine */
            template <>
            struct bpu_list_traits<void> : public std::true_type {};

            /* non-void must satisfy bpu-list rules */
            template <typename BpuList>
            struct bpu_list_traits {
                /* checks everything except BpuList::rest_type */
                static constexpr bool _value1 = bpu_node_concept<BpuList>;
                static constexpr bool _value2 = bpu_list_traits<typename BpuList::rest_type>::value;

                static constexpr bool value = (_value1 && _value2);
            };

            // ----------------------------------------------------------------

            template <typename BpuList>
            constexpr bool bpu_list_v = bpu_list_traits<BpuList>::value;
        }

        /* may want to rename this -> native_bpu_list */
        template <typename BpuList>
        concept bpu_list_concept = detail::bpu_list_v<BpuList>;

        // ----------------------------------------------------------------

        /* TODO: retire in favor of unit_concept? */
        template <typename Dimension>
        concept dimension_concept = requires(Dimension dim)
        {
            typename Dimension::dim_type;
            typename Dimension::canon_type;
        }
            && (bpu_list_concept<typename Dimension::dim_type>
                && bpu_list_concept<typename Dimension::canon_type>
                );

    } /*namespace unit*/
} /*namespace xo*/

/* end dimension_concept.hpp */
