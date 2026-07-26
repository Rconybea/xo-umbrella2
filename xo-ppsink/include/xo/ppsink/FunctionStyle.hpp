/** @file FunctionStyle.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <string_view>
#include <cstdint>

namespace xo {
    enum class FunctionStyle : std::uint8_t {
        /** literal: print supplied text as-is **/
        literal,
        /** pretty: print name, surrounded by [], preserve detail
         *   [double Quadratic::evaluate(double) const]
         **/
        pretty,
        /** streamlined: keep just class::method
         *   Quadratic::evaluate
         **/
        streamlined,
        /** simple: keep just method name
         *   evaluate
         **/
        simple,
    };

    /** string description for @p x **/
    constexpr std::string_view
    descr_of(FunctionStyle x) {
        switch(x) {
        case FunctionStyle::literal: return "literal";
        case FunctionStyle::pretty: return "pretty";
        case FunctionStyle::streamlined: return "streamlined";
        case FunctionStyle::simple: return "simple";
        }

        return "?FunctionStyle";
    }
}

/* end FunctionStyle.hpp */
