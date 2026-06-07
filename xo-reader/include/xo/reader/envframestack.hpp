/* file envframestack.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "xo/expression/LocalSymtab.hpp"

namespace xo {
    namespace scm {
        /** @class envframestack
         *  @brief A stack of envframe objects
         **/
        class envframestack {
        public:
            using ppstate      = xo::print::ppstate;
            using ppindentinfo = xo::print::ppindentinfo;

        public:
            envframestack() {}

            bool empty() const { return stack_.empty(); }
            std::size_t size() const { return stack_.size(); }

            /** lookup variable in environment stack.
             *  Visit frames in fifo order,  report first match;
             *  nullptr if no matches.
             **/
            bp<Expression> lookup(const std::string & x) const;

            /** update/replace binding for variable @p target.
             *  New binding may have a different type.
             **/
            void upsert(bp<Variable> target);

            bp<SymbolTable> top_envframe() const;
            void push_envframe(const rp<SymbolTable> & x);
            rp<SymbolTable> pop_envframe();

            void reset_to_toplevel() { stack_.resize(1); }

            /** relative to top-of-stack.
             *  0 -> top (last in),  z-1 -> bottom (first in)
             **/
            bp<SymbolTable> operator[](std::size_t i) {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1].get();
            }

            bp<SymbolTable> operator[](std::size_t i) const {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1].get();
            }

            void print (std::ostream & os) const;
            bool pretty_print(const ppindentinfo & ppii) const;

        private:
            std::vector<rp<SymbolTable>> stack_;
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
