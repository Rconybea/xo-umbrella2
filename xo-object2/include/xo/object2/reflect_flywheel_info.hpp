/** @file reflect_flywheel_info.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

namespace xo {
    namespace facet {
        /** describe @ref FlywheelInfo and its parts to xo-reflect, so a frame
         *  can be rendered as json.
         *
         *  ## Why this lives in xo-object2, which does not own the type
         *
         *  It has no better home yet, and the reason is levelization rather
         *  than preference.  @c AllocFlywheel belongs to xo-facet, but xo-facet
         *  cannot reach a StructReflector:
         *
         *  @code
         *  xo-deps --why=xo-printjson:xo-facet   # xo-printjson -> xo-printable2 -> xo-facet
         *  @endcode
         *
         *  so the convention DFloat and DString follow -- a type's own
         *  subsystem registers its reflection and printer -- is unavailable
         *  here.  Of the subsystems that CAN, xo-reflectable2 is topologically
         *  lowest with {facet, reflect} and xo-stringtable2 lowest with
         *  printjson as well; xo-object2 is simply the first whose subject is
         *  not actively wrong, and the first where this and its test can sit
         *  together.
         *
         *  **Expected to move.**  The intended consumer is a websocket feeding
         *  a browser animation, and when that lands this belongs beside it.
         *  Kept as a free function in its own file for exactly that reason:
         *  moving it should be moving a file, not disentangling it from
         *  SetupObject2.
         *
         *  Idempotent -- StructReflector's completion flag is per-type and
         *  static, so calling this twice is harmless.
         **/
        void reflect_flywheel_info();
    } /*namespace facet*/
} /*namespace xo*/

/* end reflect_flywheel_info.hpp */
