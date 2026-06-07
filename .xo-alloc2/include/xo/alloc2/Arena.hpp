/** @file Arena.hpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

// reminder: we can't put this AAllocator
// (or APrintable for that matter) support in xo-arena,
// because xo-arena is a dependency of xo-facet, which is in turn
// a dependency of xo-alloc2
//

#include <xo/arena/DArena.hpp>
#include "arena/IAllocator_DArena.hpp"

/* end Arena.hpp */
