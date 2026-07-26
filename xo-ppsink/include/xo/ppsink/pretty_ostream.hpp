/** @file pretty_ostream.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  pretty() together with the <ostream> its operator<< fallback needs.
 *
 *  Anyone calling pretty() (in particular a Prettifier implementor that
 *  renders members via pretty()) includes one of:
 *   - pretty.hpp          -- when every value reaches a Prettifier<T> or a
 *                            string-like leaf, so the operator<< fallback is
 *                            never instantiated (ostream-free), or
 *   - pretty_ostream.hpp  -- when some value is an opaque leaf rendered via
 *                            operator<<, which needs <ostream> visible at the
 *                            point pretty<T> is instantiated.
 **/

#pragma once

#include "pretty.hpp"
#include <ostream>

/* end pretty_ostream.hpp */
