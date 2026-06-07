/* file DynamicEndpoint.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "DynamicEndpoint.hpp"

namespace xo {
    using xo::web::Alist;
    using xo::fn::CallbackId;

    namespace web {
        DynamicEndpoint::DynamicEndpoint(std::string uri_pattern,
                                         HttpEndpointFn http_fn,
                                         StreamSubscribeFn subscribe_fn,
                                         StreamUnsubscribeFn unsubscribe_fn)
            : uri_pattern_{std::move(uri_pattern)},
              http_fn_{std::move(http_fn)},
              subscribe_fn_{std::move(subscribe_fn)},
              unsubscribe_fn_{std::move(unsubscribe_fn)}
        {
            std::string r_pat;

            /* 1st pass -- construct pattern regex .uri_regex
             * to identify urls that belong to this endpoint
             *
             * using regex like:
             *   \$\{[[:alnum:]]+\}
             */
            {
                std::regex var_rgx("\\$\\{[[:alnum:]]+\\}");

                /* e.g. if .uri_pattern:
                 *    /fixed/stem/${a}/more/fixed/stuff/${b}
                 * then want r_pat:
                 *    /fixed/stem/[[:alnum:]]+/more/fixed/stuff/[[:alnum:]]+
                 * to find values pattern variables like ${a}, ${b}
                 */
                std::regex_replace(std::back_inserter(r_pat),
                                   this->uri_pattern_.begin(),
                                   this->uri_pattern_.end(),
                                   var_rgx,
                                   std::string("([[:alnum:]]+)"));

                this->uri_regex_ = std::regex(r_pat);
            }

            /* 2nd pass -- identify pattern variables */
            {
                /* regex for:
                 *   \$\{([[:alnum:]]+)\}
                 * use to match input like
                 *   ${apple}
                 * and also extract the variable name
                 *   apple
                 */
                std::regex var_rgx("\\$\\{([[:alnum:]]+)\\}");
                std::smatch match;

                std::string subject = this->uri_pattern_;

                /* if subject like
                 *   /fixed/stem/${a}/more/fixed/stuff/${b}
                 * extract
                 *   ["a", "b"]
                 *
                 * for
                 *   /fixed/stem/${a}/more/fixed/stuff/${b}/${a}
                 * also extract
                 *   ["a", "b"]
                 * i.e. avoid extracting the same variable name twice
                 */
                while (std::regex_search(subject, match, var_rgx)) {
                    std::string v = match[1];

                    bool present_flag = false;

                    for (auto const & x : this->var_v_) {
                        if (x == v) {
                            present_flag = true;
                            break;
                        }
                    }

                    if (!present_flag)
                        this->var_v_.push_back(match[1]);

                    subject = match.suffix().str();
                }
            }
        } /*ctor*/

        void
        DynamicEndpoint::http_response(std::string const & incoming_uri,
                                       std::ostream * p_os) const
        {
            /* send this uri argument list  callback.
             * contains variables extracted from .uri_pattern
             * (variables surrounded by ${...})
             */
            Alist alist;

            /* extract pattern variables in uri
             * c.f. 2nd pass in DynamicEndpoint.ctor
             */
            std::smatch match;
            std::string subject = incoming_uri;

            /* if subject like
             *   /fixed/stem/apple/more/fixed/stuff/beagle
             * with .uri_pattern
             *   /fixed/stem/${a}/more/fixed/stuff/${b}
             * then we have .uri_regex
             *   /fixed/stem/([[:alnum:]]+)/more/fixed/stuff/([[:alnum:]]+)
             * use this to extract values for keys in .var_v,
             * in the same order
             */
            if (std::regex_match(subject, match, this->uri_regex_)) {
                for (size_t i = 0, n = this->var_v_.size(); i<n; ++i) {
                    std::string i_name = this->var_v_[i];
                    std::string i_value = match[1+i];

                    alist.push_back(i_name, i_value);
                }
            }

            this->http_fn_(incoming_uri, alist, p_os);
        } /*http_response*/

        CallbackId
        DynamicEndpoint::subscribe(std::string const & /*incoming_uri*/,
                                   rp<AbstractSink> const & ws_sink) const
        {
            return this->subscribe_fn_(ws_sink);
        } /*subscribe*/

        void
        DynamicEndpoint::unsubscribe(CallbackId id) const
        {
            return this->unsubscribe_fn_(id);
        } /*unsubscribe*/
    } /*namespace web*/
} /*namespace xo*/

/* end DynamicEndpoint.cpp */
