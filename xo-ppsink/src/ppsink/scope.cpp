/** @file scope.cpp **/

#include <xo/ppsink/scope.hpp>
#include <string>

namespace xo::print {
    void
    scope::emit_indent(xo::print::LogState & st) {
        std::string pad(st.nesting_level() * scope_config::indent_width, ' ');
        st.sink().put(pad);
    }
} /*namespace xo::print*/

/* end scope.cpp */
