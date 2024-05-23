/** @file quantity_ops.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity_concept.hpp"

namespace xo {
    namespace qty {
        /** @defgroup quantity-dimensionless-operators **/
        ///@{

        /** subtract an arithmetic value from a dimensionless quantity **/
        template <typename Quantity,
                  typename Dimensionless>
        requires (quantity_concept<Quantity>
                  && Quantity::is_dimensionless()
                  && std::is_arithmetic_v<Dimensionless>)
        constexpr auto
        operator- (const Quantity & x, Dimensionless y)
        {
            using repr_type = std::common_type_t<typename Quantity::repr_type, Dimensionless>;

            auto xp = static_cast<repr_type>(x.scale());
            auto yp = static_cast<repr_type>(y);

            return xp - yp;
        }

        /** subtract a dimensionless quantity from an arithmetic value **/
        template <typename Dimensionless,
                  typename Quantity>
        requires (std::is_arithmetic_v<Dimensionless>
                  && quantity_concept<Quantity>
                  && Quantity::is_dimensionless())
        constexpr auto
        operator- (Dimensionless x, const Quantity & y)
        {
            using repr_type = std::common_type_t<Dimensionless, typename Quantity::repr_type>;

            auto xp = static_cast<repr_type>(x);
            auto yp = static_cast<repr_type>(y.scale());

            return xp - yp;
        }

        ///@}

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires quantity_concept<Quantity> && quantity_concept<Quantity2>
        constexpr auto
        operator== (const Quantity & x, const Quantity2 & y)
        {
            return (Quantity::compare(x, y) == 0);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires quantity_concept<Quantity> && quantity_concept<Quantity2>
        constexpr auto
        operator<=> (const Quantity & x, const Quantity2 & y)
        {
            return Quantity::compare(x, y);
        }

    } /*namespace qty*/

} /*namespace xo*/


/** end quantity_ops.hpp **/
