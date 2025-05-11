/* file envframestack.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "envframe.hpp"

namespace xo {
    namespace scm {
        /** @class envframestack
         *  @brief A stack of envframe objects
         **/
        class envframestack {
        public:
            using Variable = xo::ast::Variable;

        public:
            envframestack() {}

            bool empty() const { return stack_.empty(); }
            std::size_t size() const { return stack_.size(); }

            /** lookup variable in environment stack.
             *  Visit frames in fifo order,  report first match;
             *  nullptr if no matches.
             **/
            rp<Variable> lookup(const std::string & x) const;

            envframe & top_envframe();
            void push_envframe(envframe x);
            void pop_envframe();

            /** relative to top-of-stack.
             *  0 -> top (last in),  z-1 -> bottom (first in)
             **/
            envframe & operator[](std::size_t i) {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1];
            }

            const envframe & operator[](std::size_t i) const {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1];
            }

            void print (std::ostream & os) const;

        private:
            std::vector<envframe> stack_;
        };

        inline std::ostream &
        operator<< (std::ostream & os, const envframestack & x) {
            x.print(os);
            return os;
        }

        inline std::ostream &
        operator<< (std::ostream & os, const envframestack * x) {
            if (x)
                x->print(os);
            else
                os << "nullptr";
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end envframestack.hpp */
