/** @file scope.cpp **/

#include <xo/ppsink/scope.hpp>
#include <string>

namespace xo::pp {
    void
    scope::emit_indent(xo::pp::LogState & st) {
        std::string pad(st.nesting_level() * scope_config::indent_width, ' ');
        st.sink().put(pad);
    }
} /*namespace xo::pp*/

/* end scope.cpp */
