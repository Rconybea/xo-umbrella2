/* file envframe.cpp
 *
 * author: Roland Conybeare
 */

#include "envframe.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    using xo::scm::Variable;

    namespace scm {
#ifdef OBSOLETE
        envframe::envframe(const std::vector<rp<Variable>> & argl,
                           const rp<Environment>& parent_env)
        {
            this->env_ = LocalEnv::make(argl, parent_env);
        }

        bp<Variable>
        envframe::lookup(const std::string & target) const {
            return env_->lookup_local(target);
        }

        void
        envframe::upsert(bp<Variable> target) {
            env_->upsert_local(target);
        }

        void
        envframe::print(std::ostream & os) const {
            os << "<envframe"
               << xtag("argv", env_->argv())
               << ">";
        }
#endif
    } /*namespace scm */
} /*namespace xo*/


/* end envframe.cpp */
