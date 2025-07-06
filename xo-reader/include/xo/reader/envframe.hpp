/* file envframe.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

xxx;

#include "xo/expression/Variable.hpp"
#include "xo/expression/LocalEnv.hpp"
#include <vector>

namespace xo {
    namespace scm {
#ifdef OBSOLETE
        /** @class envframe
         *  @brief names/types of formal paremeters introduced by a function
         *
         *
         **/
        class envframe {
        public:
            using Environment = xo::ast::Environment;
            using LocalEnv    = xo::ast::LocalEnv;
            using Variable    = xo::ast::Variable;

        public:
            envframe() = default;
            envframe(const std::vector<rp<Variable>> & argl, const rp<Environment> & parent_env);

            const std::vector<rp<Variable>> & argl() const { return env_->argv(); }
            const rp<LocalEnv> & local_env() const { return env_; }

            /** lookup variable by @p name.  If found, return it.
             *  Otherwise return nullptr
             **/
            bp<Variable> lookup(const std::string & name) const;

            /** establish (replacing if already exists) binding for variable @p var.
             *  Replacement intended for use in interactive sessions
             **/
            void upsert(bp<Variable> var);

            void print (std::ostream & os) const;

        private:
            rp<LocalEnv> env_;
        };

        inline std::ostream &
        operator<< (std::ostream & os, const envframe & x) {
            x.print(os);
            return os;
        }
#endif
    } /*namespace scm*/
} /*namespace xo*/

/* end envframe.hpp */
