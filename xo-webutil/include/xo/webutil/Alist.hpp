/* file Alist.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include <vector>
#include <string>
#include <string_view>

namespace xo {
    namespace web {
        /* assocation list, maps strings to strings
         * use this for arguments to dynamic-endpoint-callbacks
         */
        class Alist {
        public:
            Alist() = default;

            /* lookup association by name */
            std::string_view lookup(std::string n) const;

            void push_back(std::string n, std::string v);

        private:
            std::vector<std::pair<std::string, std::string>> assoc_v_;
        }; /*Alist*/

    } /*namespace web*/
} /*namespace xo*/

/* end Alist.hpp */
