/* file DynamicEndpoint.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "EndpointUtil.hpp"
#include "WebsocketSink.hpp"
#include <xo/webutil/Alist.hpp>
#include <xo/webutil/HttpEndpointDescr.hpp>
#include <xo/webutil/StreamEndpointDescr.hpp>
#include <xo/refcnt/Refcounted.hpp>
#include <regex>

namespace xo {
    namespace web {
        /* a dynamic http endpoint.  content served on-browser-demand
         * by user-provided callback
         *
         * Reference-counted since 2026-09-26.  The webserver's endpoint map
         * holds one, and so does every websocket subscription made through it:
         * a subscription must later call THIS endpoint's unsubscribe, to detach
         * its sink from the source.  Before, the map held the only owner
         * (unique_ptr) and subscriptions a raw pointer, so re-registering a
         * stem freed the endpoint out from under live subscriptions.  Now the
         * old endpoint lives until its last subscription ends.  First step of
         * .xo-backlog/xo-websock/issues/07.
         */
        class DynamicEndpoint : public ref::Refcount {
        public:
            using CallbackId = fn::CallbackId;

        public:
            static rp<DynamicEndpoint> make_http(std::string uri_pattern,
                                                 HttpEndpointFn http_cb) {
                return (rp<DynamicEndpoint>
                        (new DynamicEndpoint(std::move(uri_pattern),
                                             std::move(http_cb),
                                             nullptr,
                                             nullptr,
                                             nullptr)));
            } /*make_http*/

            /* @p recv_fn optional; see StreamReceiveFn */
            static rp<DynamicEndpoint> make_stream(std::string uri_pattern,
                                                   StreamSubscribeFn sub_fn,
                                                   StreamUnsubscribeFn unsub_fn,
                                                   StreamReceiveFn recv_fn = nullptr) {
                return (rp<DynamicEndpoint>
                        (new DynamicEndpoint(std::move(uri_pattern),
                                             nullptr,
                                             std::move(sub_fn),
                                             std::move(unsub_fn),
                                             std::move(recv_fn))));
            } /*make_stream*/

            std::string stem() const {
                return EndpointUtil::stem(this->uri_pattern_);
            } /*stem*/

#ifdef NOT_USING
            /* true iff incoming_uri matches .uri_pattern */
            bool is_match(std::string const & incoming_uri) const {
                /* c++ regex = javascript regexes,
                 * so these characters are special:
                 *   ^ $ \ . * + ? ( ) [ ] { } |
                 */
            } /*is_match*/
#endif

            /* get html from this endpoint,  on behalf of uri=incoming_uri;
             * write html on *p_os
             *
             * require: non-null http_fn
             */
            void http_response(std::string const & incoming_uri,
                               std::ostream * p_os) const;

            /* subscribe stream from this endpoint,  on behalf of uri=incoming_uri.
             * send output to ws_sink
             */
            CallbackId subscribe(std::string const & incoming_uri,
                                 rp<WebsocketSink> const & ws_sink) const;

            /* unsubscribe stream from this endpoint;
             * reverses the effect of a previous call to .subscribe()
             * that returned id
             */
            void unsubscribe(CallbackId id) const;

            /* true iff this endpoint accepts {"cmd": "send", ...} */
            bool has_receive() const { return static_cast<bool>(receive_fn_); }

            /* deliver application message @p msg, sent by the subscriber
             * whose sink is @p ws_sink.  See StreamReceiveFn for the
             * threading contract.
             *
             * require: has_receive()
             */
            void receive(rp<WebsocketSink> const & ws_sink,
                         Json::Value const & msg) const;

        private:
            explicit DynamicEndpoint(std::string uri_pattern,
                                     HttpEndpointFn http_fn,
                                     StreamSubscribeFn subscribe_fn,
                                     StreamUnsubscribeFn unsubscribe_fn,
                                     StreamReceiveFn receive_fn);

        private:
            /* pattern for this endpoint
             * can be string like
             *   /fixed/stem/${a}/more/fixed/stuff/${b}
             * in which case:
             *
             * 1. will match uris like:
             *     /fixed/stem/apple/more/fixed/stuff/bananas
             *    --> invoke callback with Alist
             *        ("a" -> "apple", "b" -> "bananas")
             *    endpoint will be stored in WebserverImpl.stem_map
             *    under fixed prefix,  in this case
             *     /fixed/stem/
             *
             * 2. will not match uris like:
             *     /fixed/stem/app/le/more/fixed/stuff/bononos
             */
            std::string uri_pattern_;
            /* regex for matching input that satisfies .uri_pattern:
             * each occurrence of
             *    ${...} replaced by [[:alnum:]]+
             */
            std::regex uri_regex_;
            /* variables found in .uri_pattern,
             * in the order in which they appear
             * if .uri_pattern is
             *   /fixed/stem/${a}/more/fixed/stuff/${b}
             * then .var_v will be:
             *   ["a", "b"]
             */
            std::vector<std::string> var_v_;
            /* run this function to produce an http response */
            HttpEndpointFn http_fn_;
            /* run this function to subscribe event stream */
            StreamSubscribeFn subscribe_fn_;
            /* run this function to unsubscribe event stream */
            StreamUnsubscribeFn unsubscribe_fn_;
            /* run this function on {"cmd": "send", ...}; may be empty */
            StreamReceiveFn receive_fn_;
        }; /*DynamicEndpoint*/

    } /*namespace web*/
} /*namespace xo*/

/* end DynamicEndpoint.hpp */
