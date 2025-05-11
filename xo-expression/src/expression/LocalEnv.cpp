/* file LocalEnv.cpp
 *
 * author: Roland Conybeare
 */

#include "LocalEnv.hpp"

namespace xo {
    namespace ast {
        binding_path
        LocalEnv::lookup_local_binding(const std::string & vname) const
        {
            int j_slot = 0;
            for (const auto & arg : argv_) {
                if (arg->name() == vname)
                    return { 0 /*i_link*/, j_slot };
                ++j_slot;
            }

            return { -2 /*i_link: sentinel*/, 0 };
        } /*lookup_local_binding*/

        binding_path
        LocalEnv::lookup_binding(const std::string & vname) const
        {
            {
                auto local = this->lookup_local_binding(vname);

                if (local.i_link_ == 0)
                    return local;
            }

            auto free = parent_env_->lookup_binding(vname);

            if (free.i_link_ == -1)
                return free;
            else
                return { free.i_link_ + 1, free.j_slot_ };
        } /*lookup_binding*/
    } /*namespace ast*/
} /*namespace xo*/


/* end LocalEnv.cpp */
