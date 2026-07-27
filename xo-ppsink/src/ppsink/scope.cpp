/** @file scope.cpp **/

#include <xo/ppsink/scope.hpp>
#include <string>

namespace xo::pp {
    void
    scope::emit_indent(xo::pp::LogState & st) {
        std::uint32_t n = st.nesting_level() * scope_config::indent_width;
        if (n > scope_config::max_indent_width)
            n = scope_config::max_indent_width;   /* cap deep nesting */
        std::string pad(n, ' ');
        st.sink().put(pad);
    }
} /*namespace xo::pp*/

/* end scope.cpp */
