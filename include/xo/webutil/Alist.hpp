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
      std::string_view lookup(std::string n) const {
	for (auto const & ix : this->assoc_v_) {
	  if (ix.first == n) {
	    return ix.second;
	  }
	}

	return "";
      } /*lookup*/

      void push_back(std::string n, std::string v) {
	this->assoc_v_.push_back(std::make_pair(std::move(n), std::move(v)));
      }

    private:
      std::vector<std::pair<std::string, std::string>> assoc_v_;
    }; /*Alist*/
    
  } /*namespace web*/
} /*namespace xo*/

/* end Alist.hpp */
