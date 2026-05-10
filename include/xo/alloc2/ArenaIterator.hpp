/** @file ArenaIterator.hpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#pragma once

// reminder: we can't put this AAllocIterator support in xo-arena
// because xo-arena is a dependency of xo-facet, which we're relying
// on here

#include <xo/arena/DArenaIterator.hpp>
#include "arena/IAllocIterator_DArenaIterator.hpp"

/* end ArenaIterator.hpp */
