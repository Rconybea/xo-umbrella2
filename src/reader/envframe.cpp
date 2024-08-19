/* file envframe.cpp
 *
 * author: Roland Conybeare
 */

#include "envframe.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    using xo::ast::Variable;

    namespace scm {
        rp<Variable>
        envframe::lookup(const std::string & x) const {
            for (const auto & var : argl_) {
                if (x == var->name())
                    return var;
            }

            return nullptr;
        }

        void
        envframe::print(std::ostream & os) const {
            os << "<envframe"
               << xtag("argl", argl_)
               << ">";
        }

    } /*namespace scm */
} /*namespace xo*/


/* end envframe.cpp */
