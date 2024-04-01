/* @file ratio_concept.hpp */

#pragma once

#include <concepts>

namespace xo {
    namespace obs {
        template <typename Ratio>
        concept ratio_concept = (std::is_signed_v<decltype(Ratio::num)>
                                 && std::is_signed_v<decltype(Ratio::den)>);
    } /*namespace obs*/
} /*namespace xo*/
