/** @file appcx_indentlog2.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include "config_indentlog2.hpp"
#include "TempArena.hpp"

namespace xo {
    /** There's some configuration that logically belongs to indentlog2/,
     *  but is stored in thread-local storage.
     *
     *  This is _not_ a model to use as a general-purpose pattern.
     **/
    class Indentlog2_Appcx {
    public:
        using TempArena = xo::mm::TempArena;

    public:
        Indentlog2_Appcx(const Indentlog2_Config & cfg);

        TempArena & temp_arena() { return temp_arena_; }

    private:
        /** xo-indentlog2/ configuration **/
        Indentlog2_Config config_;

        /** temporary arena storage **/
        TempArena temp_arena_;
    };
} /*namespace xo*/

/* end appcx_indentlog2.hpp */
