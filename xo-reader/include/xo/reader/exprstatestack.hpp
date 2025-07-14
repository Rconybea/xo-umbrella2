/* file exprstatestack.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"
#include "xo/indentlog/print/vector.hpp"
#include "xo/indentlog/print/pretty.hpp"

namespace xo {
    namespace scm {
        /** @class exprstatestack
         *  @brief A stack of exprstate objects
         **/
        class exprstatestack {
        public:
            exprstatestack() {}

            bool empty() const { return stack_.empty(); }
            std::size_t size() const { return stack_.size(); }

            exprstate & top_exprstate();
            void push_exprstate(std::unique_ptr<exprstate> exs);
            std::unique_ptr<exprstate> pop_exprstate();

            /** relative to top-of-stack.
             *  0 -> top (last in),  z-1 -> bottom (first in)
             **/
            std::unique_ptr<exprstate> & operator[](std::size_t i) {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1];
            }

            const std::unique_ptr<exprstate> & operator[](std::size_t i) const {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1];
            }

            void print (std::ostream & os) const;

        private:
            std::vector<std::unique_ptr<exprstate>> stack_;
        };

        inline std::ostream &
        operator<< (std::ostream & os, const exprstatestack & x) {
            x.print(os);
            return os;
        }

        inline std::ostream &
        operator<< (std::ostream & os, const exprstatestack * x) {
            if (x)
                x->print(os);
            else
                os << "nullptr";
            return os;
        }
    } /*namespace scm*/

#ifndef ppdetail_atomic
    namespace print {
        template <>
        struct ppdetail<xo::scm::exprstatestack *> {
            static bool print_upto(ppstate * pps, const xo::scm::exprstatestack * x) {
                return ppdetail_atomic<const xo::scm::exprstatestack *>::print_upto(pps, x);
            }
            static void print_pretty(ppstate * pps, const xo::scm::exprstatestack * x) {
                ppdetail_atomic<const xo::scm::exprstatestack *>::print_pretty(pps, x);
            }
        };
    }
#endif

} /*namespace xo*/


/* end exprstatestack.hpp */
