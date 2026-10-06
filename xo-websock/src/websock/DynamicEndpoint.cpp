/* file DynamicEndpoint.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "DynamicEndpoint.hpp"
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflect/EnumReflector.hpp>
#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace xo {
    using xo::reflect::StructReflector;
    using xo::web::Alist;
    using xo::fn::CallbackId;

    namespace web {
        namespace {
            /** compile uri pattern @p pattern: a regex matching the uris
             *  it covers, and its variables in order.
             *
             *    /fixed/stem/${a}/more/${b}  ->  /fixed/stem/([^/]+)/more/([^/]+)
             *    /src/${path...}             ->  /src/(.+)
             *
             *  A variable matches one path segment; the LAST may be written
             *  ${name...} to match the rest of the uri, slashes included.
             *  Fixed text matches itself (regex characters escaped).
             **/
            void
            compile_pattern(std::string const & pattern,
                            std::regex * p_regex,
                            std::vector<std::string> * p_var_v)
            {
                static std::regex const var_rgx("\\$\\{([[:alnum:]_]+)(\\.\\.\\.)?\\}");
                static std::regex const special_rgx("[.^$|()\\[\\]{}*+?\\\\]");

                std::string r_pat;
                std::string::const_iterator pos = pattern.begin();
                std::smatch match;

                auto literal = [&r_pat](std::string const & text) {
                    r_pat += std::regex_replace(text, special_rgx, "\\$&");
                };

                while (std::regex_search(pos, pattern.end(), match, var_rgx)) {
                    literal(std::string(pos, match[0].first));

                    bool rest = match[2].matched;

                    if (rest && (match[0].second != pattern.end()))
                        throw std::invalid_argument("uri pattern [" + pattern + "]: ${"
                                                    + match[1].str()
                                                    + "...} must end the pattern");

                    r_pat += (rest ? "(.+)" : "([^/]+)");

                    std::string v = match[1].str();

                    /* a variable repeated in the pattern is reported once, at
                     * its first position
                     */
                    if (std::find(p_var_v->begin(), p_var_v->end(), v) == p_var_v->end())
                        p_var_v->push_back(v);

                    pos = match[0].second;
                }

                literal(std::string(pos, pattern.end()));

                *p_regex = std::regex(r_pat);
            } /*compile_pattern*/
        } /*namespace*/

        DynamicEndpoint::DynamicEndpoint(EndpointKind kind,
                                         std::string uri_pattern,
                                         HttpHandler http_handler,
                                         StreamSubscribeFn subscribe_fn,
                                         StreamUnsubscribeFn unsubscribe_fn,
                                         rp<StreamReceiver> receiver)
            : kind_{kind},
              uri_pattern_{std::move(uri_pattern)},
              http_handler_{std::move(http_handler)},
              subscribe_fn_{std::move(subscribe_fn)},
              unsubscribe_fn_{std::move(unsubscribe_fn)},
              receiver_{std::move(receiver)}
        {
            compile_pattern(this->uri_pattern_, &this->uri_regex_, &this->var_v_);
        } /*ctor*/

        HttpResponse
        DynamicEndpoint::http_response(std::string const & incoming_uri) const
        {
            assert(this->kind_ == EndpointKind::http);

            /* the router finds this endpoint by its fixed stem; the whole
             * uri must still match the pattern.  e.g. .uri_pattern
             *   /fixed/stem/${a}
             * is found for /fixed/stem/x/y, which it does not match
             */
            std::smatch match;

            if (!std::regex_match(incoming_uri, match, this->uri_regex_))
                return HttpResponse::not_found("no endpoint matches [" + incoming_uri + "]");

            /* each variable's value, in .var_v order.  A repeated variable
             * takes its first occurrence's value
             */
            Alist vars;

            for (size_t i = 0, n = this->var_v_.size(); i < n; ++i)
                vars.push_back(this->var_v_[i], match[1 + i]);

            return this->http_handler_(HttpRequest(incoming_uri, std::move(vars)));
        } /*http_response*/

        CallbackId
        DynamicEndpoint::subscribe(std::string const & /*incoming_uri*/,
                                   rp<WebsocketSink> const & ws_sink) const
        {
            assert(this->kind_ == EndpointKind::stream);

            return this->subscribe_fn_(ws_sink);
        } /*subscribe*/

        void
        DynamicEndpoint::unsubscribe(CallbackId id) const
        {
            assert(this->kind_ == EndpointKind::stream);

            return this->unsubscribe_fn_(id);
        } /*unsubscribe*/

        void
        DynamicEndpoint::receive(rp<WebsocketSink> const & ws_sink,
                                 Json::Value const & msg) const
        {
            assert(this->kind_ == EndpointKind::stream);
            assert(this->receiver_);

            this->receiver_->receive(ws_sink, msg);
        } /*receive*/
        void
        reflect_endpoint_kind(reflect::TypeDescrTable * /*table*/)
        {
            reflect::EnumReflector<EndpointKind> er;

            if (er.is_incomplete()) {
                REFLECT_ENUM(er, http);
                REFLECT_ENUM(er, stream);
            }
        } /*reflect_endpoint_kind*/

        void
        DynamicEndpoint::reflect_self(reflect::TypeDescrTable * /*table*/)
        {
            StructReflector<DynamicEndpoint> sr;

            if (sr.is_incomplete()) {
                /* not uri_regex_ or the std::functions: not reflectable
                 * yet; the printer summarizes them.  Not receiver_: printed
                 * in full inline by the printer -- a ref.  kind_: an enum,
                 * reflected by reflect_endpoint_kind
                 */
                REFLECT_MEMBER(sr, kind);
                REFLECT_MEMBER(sr, uri_pattern);
                REFLECT_MEMBER(sr, var_v);
            }
        } /*reflect_self*/
    } /*namespace web*/
} /*namespace xo*/

/* end DynamicEndpoint.cpp */
