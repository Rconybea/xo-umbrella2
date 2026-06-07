/* @file Alist.cpp */

#include "Alist.hpp"

namespace xo {
    namespace web {
        /* lookup association by name */
        std::string_view
        Alist::lookup(std::string n) const {
            for (auto const & ix : this->assoc_v_) {
                if (ix.first == n) {
                    return ix.second;
                }
            }

            return "";
        } /*lookup*/

        void
        Alist::push_back(std::string n, std::string v) {
            this->assoc_v_.push_back(std::make_pair(std::move(n), std::move(v)));
        }
    } /*namespace web*/
} /*namespace xo*/



/* end Alist.cpp */
