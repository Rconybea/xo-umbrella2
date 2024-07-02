/* file LocalEnv.cpp
 *
 * author: Roland Conybeare
 */

#include "LocalEnv.hpp"

namespace xo {
    namespace ast {
        binding_path
        LocalEnv::lookup_binding(const std::string & vname) const
        {
            int j_slot = 0;
            for (const auto & arg : argv_) {
                if (arg->name() == vname)
                    return { 0 /*i_link*/, j_slot };
                ++j_slot;
            }

            auto tmp = parent_env_->lookup_binding(vname);

            if (tmp.i_link_ == -1)
                return tmp;
            else
                return { tmp.i_link_ + 1, tmp.j_slot_ };
        } /*lookup_binding*/
    } /*namespace ast*/
} /*namespace xo*/


/* end LocalEnv.cpp */
