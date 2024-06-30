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
            /** create instance.  Probably only need one of these **/
            static ref::rp<GlobalEnv> make() { return new GlobalEnv(); }

            ref::brw<Expression> require_global(const std::string & vname,
                                                ref::brw<Expression> expr) {
                global_map_[vname] = expr.get();
                return expr;
            } /*require_global*/

            // ----- Environment -----

            virtual ref::brw<Expression> lookup_var(const std::string & vname) const {
                auto ix = global_map_.find(vname);

                if (ix == global_map_.end()) {
                    /* not found */
                    return ref::brw<Expression>::from_native(nullptr);
                }

                return ix->second;
            }

        private:
            GlobalEnv() = default;

        private:
            /* for assignable globals,  need to allocate memory
             * addresses for these.
             */
            std::map<std::string, ref::rp<Expression>> global_map_;
        };
    } /*namespace ast*/
} /*namespace xo*/


/* end GlobalEnv.hpp */
