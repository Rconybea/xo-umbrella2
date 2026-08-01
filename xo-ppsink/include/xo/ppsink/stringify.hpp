/** @file stringify.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  STRINGIFY(x): the token @p x as a string literal.
 *
 *  A generic preprocessor utility -- it has nothing to do with pretty-printing
 *  (nor with indented logging or ordered initialization, its other historical
 *  users).  It lives in its own header, all by itself, so it can move cheaply
 *  if xo-ppsink is ever split into a core and its printing conveniences.  For
 *  now tag.hpp includes it, so callers of the tag/logging vocabulary keep the
 *  STRINGIFY that legacy xo-indentlog's print/tag.hpp used to provide.
 *
 *  #ifndef-guarded so a translation unit that also pulls in legacy indentlog's
 *  identical `#define STRINGIFY(x) #x` doesn't see a redefinition.
 **/

#pragma once

#ifndef STRINGIFY
# define STRINGIFY(x) #x
#endif

/* end stringify.hpp */
