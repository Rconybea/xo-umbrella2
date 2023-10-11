/* file EndpointDescr.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "web_util/Alist.hpp"
#include "refcnt/Refcounted.hpp"
#include "indentlog/print/tag.hpp"
#include "indentlog/print/tostr.hpp"
#include <functional>

namespace xo {
  namespace web {
    /* a function that can deliver http content on demand. */
    using HttpEndpointFn = std::function<void (std::string const &,
                           Alist const &,
                           std::ostream *)>;

    /* describes an http endpoint --
     * this comprises:
     * - a uri pattern.
     * - a function that can deliver http content on demand
     */
    class HttpEndpointDescr {
    public:
      HttpEndpointDescr(std::string uri_pattern,
            HttpEndpointFn endpoint_fn)
    : uri_pattern_{std::move(uri_pattern)},
      endpoint_fn_{std::move(endpoint_fn)}
      {}

      std::string const & uri_pattern() const { return uri_pattern_; }
      HttpEndpointFn const & endpoint_fn() const { return endpoint_fn_; }

      void display(std::ostream & os) const {
    using xo::xtag;

    os << "<HttpEndpointDescr" << xtag("uri_pattern", uri_pattern_) << ">";
      } /*display*/

      std::string display_string() const { return xo::tostr(*this); }

    private:
      /* unique pattern in URI-space for this endpoint.
       * for example
       *    .uri_pattern = /stem/${foo}/${bar}
       * means this endpoint generates contents for uri's
       *    /stem/apple/banana
       *    /stem/aphid/green
       * but not for
       *    /stem/apple/banana/carrot
       */
      std::string uri_pattern_;
      /* a function that can construct http output on demand
       *   .endpoint_fn(uri, alist, &os)
       * writes http output to os.   output is parameterized
       * by name-value pairs in alist,  and is prepared on behalf
       * of .uri_pattern
       * alist will report name-value pairs for each variable that
       * appears in .uri_pattern (surrounded by ${..})
       */
      HttpEndpointFn endpoint_fn_;
    }; /*HttpEndpointDescr*/

    inline std::ostream &
    operator<<(std::ostream & os, HttpEndpointDescr const & x) {
      x.display(os);
      return os;
    } /*operator<<*/

  } /*namespace web*/
} /*namespace xo*/

/* end EndpointDescr.hpp */
