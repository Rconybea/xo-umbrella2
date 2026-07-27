/** @file function_name.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Render a __PRETTY_FUNCTION__ string into a PpSink, styled per FunctionStyle.
 *
 *  Pure string parsing (arena-free), ported from legacy xo-indentlog
 *  function.hpp.  Color is applied by the caller (a color_guard around the
 *  banner), so this only produces the styled name text.
 **/

#pragma once

#include "PpSink.hpp"
#include "FunctionStyle.hpp"
#include <string_view>

namespace xo::pp {
    /** write @p pretty (typically __PRETTY_FUNCTION__) to @p sink, styled per
     *  @p style:
     *   - literal     -> @p pretty verbatim
     *   - pretty      -> [pretty]
     *   - streamlined -> Class::method  (drop return type, enclosing namespaces,
     *                                    template args, and argument list)
     *   - simple      -> method         (drop everything but the method name)
     **/
    void put_function_name(PpSink & sink, xo::FunctionStyle style, std::string_view pretty);
} /*namespace xo::pp*/

/* end function_name.hpp */
