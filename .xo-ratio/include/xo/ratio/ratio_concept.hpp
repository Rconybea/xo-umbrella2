/** @file ratio_concept.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "numeric_concept.hpp"

namespace xo {
    namespace ratio {
        /* also expect:
         *   Ratio::num_type / Ratio::den_type rounds towards -inf
         */
        template <typename Ratio>
        concept ratio_concept = requires(Ratio ratio)
        {
            typename Ratio::component_type;
            typename Ratio::component_type;

            { ratio.num() } -> std::same_as<typename Ratio::component_type>;
            { ratio.den() } -> std::same_as<typename Ratio::component_type>;
        } && numeric_concept<typename Ratio::component_type>;

    } /*namespace ratio*/
} /*namespace xo*/


/** end ratio_concept.hpp **/
