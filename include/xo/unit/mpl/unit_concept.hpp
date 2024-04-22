/* @file unit_concept.hpp */

#pragma once

#include "dimension_concept.hpp"

namespace xo {
    namespace unit {
        /** @brief concept for a Unit type, suitable for use with the quantity template
         *
         *  Example:
         *  @code
         *  using namespace xo::unit;
         *  static_assert(unit_concept<units::day>);
         *  @endcode
         **/
        template <typename Unit>
        concept unit_concept = requires(Unit unit)
        {
            typename Unit::scalefactor_type;
            typename Unit::dim_type;
            typename Unit::canon_type;
        }
            && (ratio_concept<typename Unit::scalefactor_type>
                && bpu_list_concept<typename Unit::dim_type>
                && bpu_list_concept<typename Unit::canon_type>);


        /** @brief concept for a Unit type, that contains exactly one basis dimension
         *
         *  Example:
         *  @code
         *  using namespace xo::unit
         *  static_assert(basis_unit_concept<units::volatility_250d>);
         *  @endcode
         **/
        template <typename Unit>
        concept basis_unit_concept = requires(Unit unit)
        {
            typename Unit::dim_type;
            typename Unit::dim_type::rest_type;
        }
            && (std::same_as<typename Unit::dim_type::rest_type, void>)
            && (unit_concept<Unit>);
    } /*namespace unit*/
} /*namespace xo*/


/* end unit_concept.hpp */
