/* file GlobalEnv.hpp
 *
 * author: Roland Conybeare, Jun 2024
 */

#pragma once

#include "Environment.hpp"
#include <map>

namespace xo {
    namespace ast {
        class GlobalEnv : public Environment {
        public:
            ref::brw<Variable> require_global(ref::brw<Variable> var) {
                const std::string & vname = var->name();

                auto ix = var_map_.find(vname);

                if (ix == var_map_.end()) {
                    var_map_[vname] = var.get();
                    return var;
                } else {
                    return ix->second;
                }
            } /*require_global*/

            // ----- Environment -----

            virtual ref::brw<Variable> lookup_var(const std::string & vname) const {
                auto ix = var_map_.find(vname);

                if (ix == var_map_.end())
                    return ref::brw<Variable>::from_native(nullptr);

                return ix->second;
            }

        private:
            std::map<std::string, ref::rp<Variable>> var_map_;
        };
    } /*namespace ast*/
} /*namespace xo*/


/* end GlobalEnv.hpp */
