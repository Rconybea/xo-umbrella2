/** @file PpTokenType.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

namespace xo {
    namespace print {
        enum PpTokenFlags {
            k_nominal = 0,

            /** literal text (possibly containing non-printing chars,
             *  including color-escape sequences)
             **/
            k_string = 0x01,
            /** starts group of nested items (e.g. start of function args) **/
            k_begin  = 0x02,
            /** discretionary line break here,
             *  e.g. between function arguments.
             **/
            k_break  = 0x03,
            /** ends innermost incomplete group (e.g. end of function args) **/
            k_end    = 0x04,

            k_type_mask = 0x07,

            k_fits   = 0x08,
        };
    } /*namespace print*/
} /*namespace xo*/

/* end PpTokenType.hpp */
