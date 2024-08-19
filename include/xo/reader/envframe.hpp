/* file envframe.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "xo/expression/Variable.hpp"
#include <vector>

namespace xo {
    namespace scm {
        /** @class envframe
         *  @brief names/types of formal paremeters introduced by a function
         *
         *
         **/
        class envframe {
        public:
            using Variable = xo::ast::Variable;

        public:
            envframe() = default;
            envframe(const std::vector<rp<Variable>> & argl) : argl_(argl) {}

            const std::vector<rp<Variable>> & argl() const { return argl_; }

            /** lookup variable by name.  If found, return it.
             *  Otherwise return nullptr
             **/
            rp<Variable> lookup(const std::string & name) const;

            void print (std::ostream & os) const;

        private:
            std::vector<rp<Variable>> argl_;
        };

        inline std::ostream &
        operator<< (std::ostream & os, const envframe & x) {
            x.print(os);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end envframe.hpp */
