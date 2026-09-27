/* @file Webserver.cpp
 *
 * Webserver + websocket container/scaffold.
 * Originally adapted from libwebsocket example
 *
 * 19aug2022
 * This version probably overly rigid.
 * Initial goal is to adpat example application so that:
 * a. it works as a library
 * b. doesn't hijack main thread
 *
 * Subsequently,  want to wrap to demonstrate library working from within python
 *
 * In the meantime,  adopting example code as-is without looking closely
 * at globals/singletons
 *
 */

#include "Webserver.hpp"
#include "UrlRouter.hpp"
#include "DynamicEndpoint.hpp"
#include "WebsockUtil.hpp"
#include "WebsocketSink.hpp"
#include "WsSafetyToken.hpp"
#include "WsSender.hpp"
#include "WsSessionSender.hpp"
#include "WsSessionTable.hpp"
#include "cx/WebsockAppcx.hpp"
#include "webserver_json.hpp"
#include <xo/printjson/JsonPrinter.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <sstream>
#include "WsSessionRouter.hpp"
#include <xo/printjson/PrintJson.hpp>
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <regex>
#include <vector>

namespace xo {
    using xo::json::PrintJson;
    using xo::json::JsonPrinter;
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::pp::quot;
    using xo::fn::CallbackId;
    using xo::pp::scope;
    using xo::pp::xtag;

    namespace web {
        char const *
        RunstateUtil::runstate_descr(Runstate x)
        {
#    define CASE(x) case Runstate::x: return #x
            switch(x) {
                CASE(stopped);
                CASE(stop_requested);
                CASE(running);
            }
#    undef CASE

            return "???";
        } /*runstate_descr*/

        /* both websocket and appl thread can obtain this token.
         * see WebsocketSessionRecd.  Posession of this token is evidence
         * caller holds WebsocketSessionRecd.mutex
         */
        class WsSessionSafetyToken : public SafetyToken<class WsSessionSafetyToken_tag> {
        private:
            friend class WebsocketSessionRecd;

        private:
            /* only WebsocketSessionRecd should construct this
             * mutex argument present just to alert reader
             */
            WsSessionSafetyToken(std::unique_lock<std::mutex> const &) {}
        }; /*WsSessionSafetyToken*/


        namespace {
            /* one of these is created for each client connecting to us */

            struct OutputBuffer;

            /* editor bait:
             *   WebserverImpl::send_text()
             *   ws_pss
             *
             * NOTE:
             * 1. per_session_data__http instances are created by libwebsocket library.
             *    since that's implemented in C,  ctor/dtors won't be invoked for this class
             */
            struct per_session_data__minimal {
            public:
                /* output state;  allocated as bona fide c++ object */
                OutputBuffer * output_buf_;
            }; /*per_session_data__minimal*/

            /* one of these created for each message */

            /* output destined for a particular websocket.
             * 'struct msg' that folllows is a POD C struct inherited from
             * libwebsocket example code; intend to retire that.
             *
             * An OutputMsg instance sends bytes [lo..hi),
             * using as many trips as necessary
             *
             *    +---...---+---...--------+
             *    | LWS_PRE | text payload |
             *    +---...---+---...--------+
             *    ^         ^              ^
             *    .buf      .text()        text() + text.size()
             *
             *    |<---A--->|<------B------->|
             *    |<------------C----------->|
             *
             * A: (LWS_PRE bytes)           populated by ::lws_write(), in particular encodes .buf_z
             * B: (.text_z bytes)           WebserverImpl passes this range to ::lws_write()
             * C: (LWS_PRE + .text_z bytes) ::lws_write() actually sends this range
             *
             * Note trailing null isn't required,  since length is explicitly sent
             */
            struct OutputBuffer {
            public:
                OutputBuffer(uint64_t session_id) : session_id_{session_id} {}
                ~OutputBuffer() = default;

                uint64_t session_id() const { return session_id_; }
                struct lws * wsi() const { return wsi_; }

                /* non-const access required.
                 * lws_write() will prepend headers in .buf_v[0..LWS_PRE-1]
                 */
                unsigned char * text() { return &(buf_v_[LWS_PRE]); }
                unsigned char const * text() const { return &(buf_v_[LWS_PRE]); }
                size_t text_z() const { return text_z_; }

                std::string_view text_view() const {
                    return std::string_view((char const *)(this->text()),
                                            this->text_z());
                }

                bool is_busy() const { return this->sent_seq_ < this->stored_seq_; }
                bool is_idle() const { return this->sent_seq_ == this->stored_seq_; }

                void establish_wsi(struct lws * wsi) { this->wsi_ = wsi; }

                bool is_writeable(WsSafetyToken const &) const { return is_writeable_; }
                void set_is_writeable(bool x, WsSafetyToken const &) { is_writeable_ = x; }

                /* caller must hold WebsocketSessionRecd.mutex;
                 * evidenced by wsession_token
                 */
                void store_message(uint32_t msg_seq,
                                   std::string const & text,
                                   WsSessionSafetyToken const & wsession_token) {
                    scope log(XO_ENTER0_(info));

                    wsession_token.verify();

                    if (sent_seq_ != stored_seq_) {
                        log && log("store_message: attempt storing new msg_seq but sent_seq!=stored_seq",
                                   xtag("sent_seq", sent_seq_),
                                   xtag("stored_seq", stored_seq_),
                                   xtag("msg_seq", msg_seq));
                        assert(false);
                    }

                    size_t req_z = LWS_PRE + text.size();

                    if (this->buf_v_.size() < req_z)
                        this->buf_v_.resize(req_z);

                    this->text_z_ = text.size();

                    ::memcpy(&(this->buf_v_[LWS_PRE]), text.c_str(), this->text_z_);

                    log && log(xtag("buf", (void*)&(this->buf_v_[0])),
                               xtag("msg_seq", msg_seq),
                               xtag("text", text),
                               xtag("text.size", text.size()),
                               xtag("req_z", req_z));

                    this->stored_seq_ = msg_seq;
                } /*store_message*/

                int lws_write_aux(WsSafetyToken const & ws_safety_token)
                    {
                        scope log(XO_ENTER0_(info));

                        ws_safety_token.verify();

                        this->set_is_writeable(false, ws_safety_token);

                        log && log("write to websocket",
                                   xtag("wsi", (void*)wsi_),
                                   xtag("text_z", this->text_z()));
                        log && log(xtag("text", this->text_view()));

                        /* 1. notice we allowed for LWS_PRE in the payload already;
                         *    this is mandatory for LWS_WRITE_TEXT.
                         * 2. LWS_WRITE_TEXT requires valid utf-8 payload
                         * 3. lws_write() writes entire contents,  using
                         *    multiple network writes if necessary.
                         *    Application side can ignore the possibility of partial writes.
                         */
                        int m = ::lws_write(this->wsi_,
                                            this->text(),
                                            this->text_z(),
                                            LWS_WRITE_TEXT);

                        if (m < (int)this->text_z()) {
                            /* note: first time we observed this,  browser console
                             *       showed that entire message was eventually received,
                             *       (though not if we exit() before returning)
                             */
                            lwsl_user("lws_write_aux: PARTIAL WRITE: session=[%llu], m=lws_write(z) with m<z m=[%d] z=[%lu]\n",
                                      static_cast<unsigned long long>(this->session_id_),
                                      m,
                                      this->text_z());

                            /* 23sep2022: consistent with observed behavior:
                             * - lws will write remainder of message
                             * - lws will call appl via LWS_CALLBACK_SERVER_WRITEABLE
                             *   once write has been completed
                             * according to docs lws buffers message -- if true,
                             * probably pay for message to be copied
                             */
                            return 0;
                        }

                        this->lws_write_completion(ws_safety_token);

                        return m;
                    } /*lws_write_aux*/

                /* call this after successfully sending a message */
                void lws_write_completion(WsSafetyToken const & ws_safety_token) {
                    /* session now writeable again.  either:
                     * - lws_write(z) successful
                     * - lws_write(z) incomplete,  followed by lws callback
                     *   with reason = LWS_CALLBACK_SERVER_WRITEABLE
                     */
                    this->set_is_writeable(true, ws_safety_token);

                    /* message completely written */
                    this->sent_seq_ = this->stored_seq_;
                } /*lws_write_completion*/

            private:
                /* identifies websocket session associated with this buffer
                 * established permanently in ctor
                 */
                /* from WsSessionTable::next_id(); never reused */
                uint64_t session_id_;

                /* opaque pointer;  owned by libwebsocket + identifies this session.
                 * established once (per websocket session) from LWS_CALLBACK_ESTABLISHED
                 */
                struct lws * wsi_ = nullptr;

                /* ::lws_write() takes responsibility for writing and buffering full message;
                 * IIU docs that means it doesn't return until full write has completed;
                 * this suggests it may also make reentrant callbacks for other sessions,
                 * while an incomplete call to lws_write() is on the stack.
                 *
                 * set .is_writeable to false during lws_write() calls,
                 * so that application threads can refrain from attempting nested
                 * lws_write() calls for the same session.
                 */
                bool is_writeable_ = false;

                /* seq# of last message sent using this buffer;  .sent_seq chases .stored_seq */
                uint32_t sent_seq_ = 0;
                /* seq# of last message stored using this buffer */
                uint32_t stored_seq_ = 0;

                /* buffer for outbound text.
                 * using the first LWS_PRE + .text_z bytes.
                 * 1st LWS_PRE bytes owned by lws library, must not touch these
                 */
                std::vector<unsigned char> buf_v_;
                size_t text_z_ = 0;
            }; /*OutputBuffer*/

            /*
             * Unlike ws, http is a stateless protocol.  This pss only exists for the
             * duration of a single http transaction.  With http/1.1 keep-alive and
             * http/2, that is unrelated to (shorter than) the lifetime of the network
             * connection.
             *
             * NOTE
             * 1. per_session_data__http instances are created by libwebsocket library.
             *    since that's implemented in C,  we need to arrange for manual initialization
             * 2. since libwebsocket implemented in C,  we don't expect auto-initialization
             *    or constructors to be invoked.
             * 3. libwebsocket gives us several alternatives for organizing resource
             *    allocation.  We use these callback reasons:
             *      LWS_CALLBACK_HTTP_BIND_PROTOCOL   for setup,    allocate .output_ss
             *      LWS_CALLBACK_HTTP_DROP_PROTOCOL   for teardown  free .output_ss
             */
            struct per_session_data__http {
                int test;
                /* store http reply in .output_str */
                std::string * output_str;
            };

            /* one of these is created for each vhost our protocol is used with
             *
             * NOTE
             * 1. per_vhost_data__minimal instances are created by libwebsocket library.
             *    since that's implemented in C,  ctors/dtors aren't used here
             *
             * editor bait: vhd
             */
            struct per_vhost_data__minimal {
                struct lws_context * context;
                struct lws_vhost * vhost;
                const struct lws_protocols * protocol;

                struct per_session_data__minimal * pss_list; /* linked-list of live pss*/

                //struct msg amsg; /* the one pending message... */
                //int current; /* the current message number we are caching */
            }; /*per_vhost_data__minimal*/
        } /*namespace*/

        class WebserverImpl;

        /* sends to one session; see WsSessionSender */
        using WsSessionSenderImpl = WsSessionSender<WebserverImpl>;

        /* bookkeeping record for a websocket session.
         * WebserverImpl (below) keeps exactly one of these
         * for each active websocket session
         */
        class WebsocketSessionRecd {
        public:
            /* @p url_router and @p sender are the session's router's view of
             * the webserver; see WsSessionRouter.  @p url_router is borrowed,
             * and must outlive this session
             */
            WebsocketSessionRecd(OutputBuffer * output_buf,
                                 UrlRouter const & url_router,
                                 rp<WsSessionSenderImpl> sender,
                                 rp<PrintJson> pjson)
                : output_buf_{output_buf},
                  sender_{sender},
                  router_{url_router, sender, std::move(pjson)}
            {
                assert(this->output_buf_);
            }

            /* this session's subscriptions, and the commands that use them */
            WsSessionRouter & router() { return router_; }

            /* session closed (or server going away): this session's sender,
             * and every sink holding it, drop anything further
             */
            void close_sender() { this->sender_->close(); }

            /* for introspection (JsonPrinter_WsSession) */
            std::uint64_t session_id() const { return this->sender_->session_id(); }
            WsSessionSenderImpl const & sender() const { return *(this->sender_.get()); }
            WsSessionRouter const & router() const { return router_; }

            bool is_output_busy() const {
                return (this->output_buf_
                        && this->output_buf_->is_busy());
            }
            bool outbound_q_empty() const { return this->outbound_q_.empty(); }

            void send_text(std::string text) {
                scope log(XO_ENTER0_(info));

                std::unique_lock<std::mutex> lock(this->mutex_);

                if (!(this->output_buf_)) {
                    log && log("ws_pss.output_buf not present -> exit");
                } else if (this->is_output_busy()) {
                    log && log("ws_pss.output_msg busy, enqueue");

                    /* previous message already in progress, enq or drop */
                    this->enqueue_text(std::move(text),
                                       WsSessionSafetyToken(lock));

                    /* this message will eventually get sent via
                     *   .lws_write_pending_traffic()
                     */
                } else {
                    /* send message now! */
                    log && log("output_msg idle, send now");

                    this->prepare_outbound_message(std::move(text),
                                                   WsSessionSafetyToken(lock));

                    /* can release lock,  won't be using for remainder of this function */
                    lock.unlock();

                    lws_context * lws_cx = ::lws_get_context(this->output_buf_->wsi());

                    /* interrupt libwebsocket event loop.
                     * will send 'wait cancelled' event to all sockets.
                     *
                     * Actually,  after testing -- looks like this sends to one wsi per protocol:
                     * basically to the "listening" wsi,  not to websocket "session" wsi
                     */
                    ::lws_cancel_service(lws_cx);

                    /* NOTE: web documentation seems to suggest using lws_callback_on_writable():
                     *
                     *   trigger call from websocket thread to send data.
                     *   will cause reentry via websocket thread into
                     *     WebserverImpl::notify_minimal()
                     *   with reason=LWS_CALLBACK_SERVER_WRITEABLE
                     *
                     * ^^^ Hmm,  doesn't seem to work this way.
                     *     Suspect this would only work if socket currently
                     *     in non-writeable state.
                     */
                    //lws_callback_on_writable(ws_pss->wsi);
                }
            } /*send_text*/

            /* write some pending traffic from lws event loop
             *
             * Require:
             * - MUST be invoked from lws event loop,  for threadsafety;
             *   ws_safety_token provides evidence of this
             */
            void lws_write_pending(WsSafetyToken const & ws_safety_token) {
                scope log(XO_ENTER0_(info));

                log && log(xtag("output_buf", (void*)this->output_buf_));

#ifdef OBSOLETE
                per_session_data__minimal * ws_pss = this->ws_pss_;

                if (!ws_pss) {
                    lscope.log("null ws_pss, exit");
                    return;
                }
#endif

                if (!(this->output_buf_)) {
                    /* output message buffer not established,
                     * implies nothing sent yet
                     */
                    log && log("output_msg either not established or destroyed, exit");
                    return;
                }

                /* loop until no queued messages for this session */
                for (;;) {
                    if (!(this->output_buf_->is_writeable(ws_safety_token))) {
                        /* call to lws_write() already in progress */
                        log && log("output_buf not writeable (bc lws_write in progress)");
                        return;
                    }

                    if (this->output_buf_->is_idle()) {
                        /* already up-to-date for this session */
                        log && log("output idle (up-to-date)");
                        return;
                    }

                    this->output_buf_->lws_write_aux(ws_safety_token);

                    /* if there are any appl messages queued,  prepare to send another one */
                    if (this->outbound_q_.empty()) {
                        /* all caught up,  nothing left to send */
                        log && log("up-to-date after write");
                    } else {
                        std::unique_lock<std::mutex> lock(this->mutex_);

                        std::string text(this->dequeue_text(WsSessionSafetyToken(lock)));

                        this->prepare_outbound_message(std::move(text),
                                                       WsSessionSafetyToken(lock));
                    }
                }
            } /*lws_write_pending*/

            /* threadsafe */
            void unsubscribe_all() {
                {
                    std::lock_guard<std::mutex> lock(this->mutex_);

                    /* drop .output_buf,
                     * to short-circuit any subsequent attempts to use .lws_write_pending()
                     * (which will happen in response to LWS_CALLBACK_EVENT_WAIT_CANCELLED
                     *  on any session)
                     */
                    this->output_buf_ = nullptr;
                }

                /* session lock NOT held: the router runs endpoints' unsubscribe
                 * functions, which may call back into the server
                 */
                this->router_.unsubscribe_all();
            } /*unsubscribe_all*/

        private:
            uint32_t generate_msg_seq() { return ++(this->last_msg_seq_); }

            /* enqueue application-level message.
             * use this when .ws_pss.outbound_buf is busy
             */
            void enqueue_text(std::string text,
                              WsSessionSafetyToken const & /*wss_token*/) {
                this->outbound_q_.push_back(std::move(text));
            } /*enqueue_text*/

            /* remove a deferred message from .outbound_q,
             * and return it.  This can happen if output becomes
             * available after being write-blocked
             */
            std::string dequeue_text(WsSessionSafetyToken const & /*wss_token*/) {
                assert(!this->outbound_q_.empty());

                std::string retval = std::move(this->outbound_q_.front());

                this->outbound_q_.pop_front();

                return retval;
            } /*dequeue_text*/

            /* prepare outbound message for sending in contiguous memory;
             * in particular prepends header.
             *
             * this can be called from either websocket or appl thread,
             * so needs to be threadsafe.
             */
            void prepare_outbound_message(std::string text,
                                          WsSessionSafetyToken const & wss_token) {
                scope log(XO_ENTER0_(info));

                /* sequence# for this outbound message */
                uint32_t msg_seq = this->generate_msg_seq();

                this->output_buf_->store_message(msg_seq,
                                                 std::move(text),
                                                 wss_token);

                /* now ws_pss->output_msg_->is_busy() */
                log && log("staged next write",
                           xtag("wsi", (void*)this->output_buf_->wsi()),
                           xtag("text_z", this->output_buf_->text_z()));
            } /*prepare_outbound_message*/

        private:
            /* output destined for this session
             * libws (via per_session_data__minimal) also points to
             * .output_buf
             */
            OutputBuffer * output_buf_ = nullptr;
            /* protects .output_buf, .last_msg_seq, .outbound_q
             * (.router has its own lock)
             */
            std::mutex mutex_;
            /* sends to this session; shared by .router and its sinks */
            rp<WsSessionSenderImpl> sender_;
            /* this session's subscriptions, and inbound command handling */
            WsSessionRouter router_;
            /* generate seq#'s for outgoing messages */
            uint32_t last_msg_seq_ = 0;
            /* when new outgoing message appears:
             * 1. if .pss->output_msg empty,  allocate it and store message there;
             *    invoke lws_callback_on_writeable(.pss->wsi) to get message sent asap
             * 2. otherwise sending a previous message is in-progress;
             *    put outgoing message to the back of .outbound_q
             */
            std::deque<std::string> outbound_q_;
        }; /*WebsocketSessionRecd*/

        /* defined in this translation unit, after WebserverImpl */
        class WebserverImplWsThread;

        class WebserverImpl : public Webserver {
            /* delivers through the protected .send_text() */
            friend class WsSessionSender<WebserverImpl>;

        public:
            WebserverImpl(WebserverConfig const & ws_config,
                          rp<PrintJson> const & pjson)
                : ws_config_{ws_config},
                  pjson_{pjson},
                  interrupt_flag_{false},
                  state_{Runstate::stopped}
                {
                } /*ctor*/

            virtual ~WebserverImpl() {
                /* if webserver is running,  initiate shutdown.
                 * webserver thread will eventually exit
                 */
                this->stop_webserver();
                /* wait for shutdown to complete */
                this->join_webserver();

                /* backstop: sessions normally close via
                 * notify_ws_session_close, but any sink the application still
                 * holds must not reach this server once it is freed
                 */
                this->session_table_.for_each([](WebsocketSessionRecd & recd)
                    {
                        recd.close_sender();
                    });
            } /*dtor*/

            virtual void run() = 0;

            // ----- Inherited from Webserver -----

            virtual Runstate state() const override { return state_; }
            virtual std::int32_t listen_port() const override { return listen_port_.load(); }
            virtual void register_http_endpoint(HttpEndpointDescr const & endpoint) override;
            virtual void register_stream_endpoint(StreamEndpointDescr const & endpoint) override;
            virtual bool unregister_http_endpoint(std::string const & uri_pattern) override;
            virtual bool unregister_stream_endpoint(std::string const & uri_pattern) override;
            virtual void visit_endpoints(EndpointVisitor const & fn) const override {
                this->url_router_.visit_endpoints(fn);
            }
            virtual void visit_sessions(SessionVisitor const & fn) const override;
            virtual void start_webserver() override;
            virtual void interrupt_stop_webserver() override;
            virtual void stop_webserver() override;
            virtual void join_webserver() override;

            /* any thread: interrupt the service thread's lws_service(), so it
             * runs LWS_CALLBACK_EVENT_WAIT_CANCELLED.  No-op when not running
             */
            void wake_service_thread();

        protected:
            void set_lws_log_level() {
                lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
                                  /* for LLL_ verbosity above NOTICE to be built into
                                   * lws, lws must have been configured and built with
                                   * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
                                  /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
                                  /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
                                  /* | LLL_DEBUG */,
                                  NULL);
            } /*set_lws_log_level*/

#if defined(LWS_WITH_PLUGINS)
            void init_pvo(lws_protocol_vhost_options * p_pvo) {
                /* {next, options, name, value} */
                *p_pvo = {NULL, NULL, "lws-minimal", ""};
            } /*init_pvo*/
#endif

            /* called once during webserver initialization;
             * identifies protocols (channels) that libws is expected to support
             */
            virtual void init_protocols(std::vector<lws_protocols> * p_v) = 0;

            void init_mount_dynamic(lws_http_mount * p_mount) {
                /* see lws-context-vhost.h for lws_http_mount */

                *p_mount = {
                    .mount_next            = NULL,
                    .mountpoint            = "/dyn",
                    .origin                = NULL,
                    .def                   = NULL,
                    .protocol              = "http",
                    .cgienv                = NULL,
                    .extra_mimetypes       = NULL,
                    .interpret             = NULL,
                    .cgi_timeout           = 0,
                    .cache_max_age         = 0,
                    .auth_mask             = 0,
                    .cache_reusable        = 0,
                    .cache_revalidate      = 0,
                    .cache_intermediaries  = 0,
#                  if (LWS_LIBRARY_VERSION_MAJOR > 4 || ((LWS_LIBRARY_VERSION_MAJOR == 4) && (LWS_LIBRARY_VERSION_MINOR >= 4)))
                    .cache_no              = 0,
#                  endif
                    .origin_protocol       = LWSMPRO_CALLBACK, /* dynamic */
                    .mountpoint_len        = 4,
                    .basic_auth_login_file = NULL,
#                  if ((LWS_LIBRARY_VERSION_MAJOR < 4) || ((LWS_LIBRARY_VERSION_MAJOR == 4) && (LWS_LIBRARY_VERSION_MINOR < 3)))
                    ._unused               = { nullptr, nullptr },
#                  endif
#                  if (LWS_LIBRARY_VERSION_MAJOR > 4 || ((LWS_LIBRARY_VERSION_MAJOR == 4) && (LWS_LIBRARY_VERSION_MINOR >= 4)))
                    .cgi_chroot_path       = NULL,
                    .cgi_wd                = NULL,
                    .headers               = NULL,
                    .keepalive_timeout     = 0,
#                  endif
                };
            } /*init_mount_dynamic*/

            void init_mount_static(lws_http_mount const * dynamic,
                                   lws_http_mount * p_mount) {
                /* serves the URL space from WebserverConfig::mount_origin
                 * (default ./mount-origin).  lws keeps the pointer: it
                 * points into .ws_config, which lives as long as we do
                 */
                *p_mount = {
                    .mount_next            = dynamic,
                    .mountpoint            = "/",
                    .origin                = this->ws_config_.mount_origin().c_str(),
                    .def                   = "index.html",
                    .protocol              = NULL,
                    .cgienv                = NULL,
                    .extra_mimetypes       = NULL,
                    .interpret             = NULL,
                    .cgi_timeout           = 0,
                    .cache_max_age         = 0,
                    .auth_mask             = 0,
                    .cache_reusable        = 0,
                    .cache_revalidate      = 0,
                    .cache_intermediaries  = 0,
#                  if (LWS_LIBRARY_VERSION_MAJOR > 4 || ((LWS_LIBRARY_VERSION_MAJOR == 4) && (LWS_LIBRARY_VERSION_MINOR >= 4)))
                    .cache_no              = 0,
#                  endif
                    .origin_protocol       = LWSMPRO_FILE,
                    .mountpoint_len        = 1,
                    .basic_auth_login_file = NULL,
#                  if ((LWS_LIBRARY_VERSION_MAJOR < 4) || ((LWS_LIBRARY_VERSION_MAJOR == 4) && (LWS_LIBRARY_VERSION_MINOR < 3)))
                    ._unused               = { nullptr, nullptr },
#                  endif
#                  if (LWS_LIBRARY_VERSION_MAJOR > 4 || ((LWS_LIBRARY_VERSION_MAJOR == 4) && (LWS_LIBRARY_VERSION_MINOR >= 4)))
                    .cgi_chroot_path       = NULL,
                    .cgi_wd                = NULL,
                    .headers               = NULL,
                    .keepalive_timeout     = 0,
#                  endif
                };
            } /*init_mount_static*/

            void init_retry(lws_retry_bo_t * p_retry) {
                p_retry->secs_since_valid_ping = 3;
                p_retry->secs_since_valid_hangup = 10;
            } /*init_retry*/

            /* requires:
             * - .pvo           initialized,  see .init_pvo()
             * - .protocol_v[]  initialized,  see .init_protocols()
             * - .mount_dynamic initialized,  see .init_mount_dynamic()
             * - .mount_static  initialized,  see .init_mount_static()
             * - .retry         initialized,  see .init_retry()
             */
            void init_cx_config(lws_context_creation_info * p_cx_config) {
                ::memset(p_cx_config, 0, sizeof(*p_cx_config));
                p_cx_config->port       = this->ws_config_.port();
                p_cx_config->vhost_name = "localhost";
#if defined(LWS_WITH_PLUGINS)
                p_cx_config->pvo        = &(this->pvo_);
#else
                p_cx_config->pvo        = nullptr;
#endif
                p_cx_config->protocols  = this->protocol_v_.data();
                p_cx_config->mounts     = &(this->mount_static_);
                /* userdata -- accessible from context with lws_context_user() */
                p_cx_config->user       = (void*)this;

#if defined(LWS_WITH_TLS)
                if (this->ws_config_.tls_flag()) {
                    lwsl_user("Server using TLS\n");
                    p_cx_config->options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
                    p_cx_config->ssl_cert_filepath = "localhost-100y.cert";
                    p_cx_config->ssl_private_key_filepath = "localhost=100y.key";
                }
#endif

                if (this->ws_config_.host_check_flag()) {
                    p_cx_config->options |= LWS_SERVER_OPTION_VHOST_UPG_STRICT_HOST_CHECK;
                }

                if (this->ws_config_.use_retry_flag()) {
                    p_cx_config->retry_and_idle_policy = &(this->retry_);
                }
            } /*init_cx_config*/

            /* write dynamic http response for incoming_uri, on *p_os
             * incoming_uri will be suffix of original uri from browser,
             * following dynamic mount point [/dyn].
             * see .init_mount_dynamic()
             */
            void dynamic_http_response(std::string const & incoming_uri,
                                       std::ostream * p_os);

            /* act on incoming websocket command, e.g.
             *   {"cmd": "subscribe", "stream": "uls"}
             *   {"cmd": "send", "sub_id": 0, "msg": <any JSON>}
             *   {"cmd": "unsubscribe", "sub_id": 0}
             * delegates to the session's WsSessionRouter; see there for replies
             */
            /* a fresh session id, never used before; see WsSessionTable */
            uint64_t next_session_id() { return this->session_table_.next_id(); }

            void perform_ws_cmd(uint64_t session_id,
                                std::string_view incoming_svw);

#ifdef DEFINED_BUT_NOT_USED
            /* called from libwebsocket thread when session manager
             * (aka "virtual host") is created for the websocket protocol
             */
            void notify_vhd(per_vhost_data__minimal * vhd);
#endif
            /* called from libwebsocket thread when creating a new websocket session */
            void notify_ws_session_open(OutputBuffer * output_buf,
                                        per_vhost_data__minimal * vhd,
                                        WsSafetyToken const & ws_safety_token);
            /* called from libwebsocket thread whenever a websocket session is closed */
            void notify_ws_session_close(OutputBuffer * output_buf,
                                         per_vhost_data__minimal * vhd,
                                         WsSafetyToken const & ws_safety_token);

            /* send text to the websocket session identified by session_id */
            void send_text(uint64_t session_id,
                           std::string text) override;

            /* from lws event loop,  write any pending outbound traffic
             * see .pending_session_q
             */
            void lws_write_pending_traffic(WsSafetyToken const & ws_safety_token);

            /* from lws event loop: end the live subscriptions of every stream
             * endpoint queued by .unregister_stream_endpoint(), in every
             * session
             */
            void lws_end_removed_subscriptions(WsSafetyToken const & ws_safety_token);

        protected:
            /* callback for http protocol */
            static int notify_dynamic_http(struct lws * wsi,
                                           lws_callback_reasons reason,
                                           void * user_data,
                                           void * incoming_uri,
                                           size_t len);

        protected:
            /* see WebserverImplWsThread below,  for methods
             * that are exclusive to libws thread
             */

            /* initial configuration for embedded webserver */
            WebserverConfig ws_config_;

            /* json printer (w/ plugins for reflected types) */
            rp<PrintJson> pjson_;

            /* --- 1. LWS configuration stuff (set once) ---*/

#if defined(LWS_WITH_PLUGINS)
            /* protocols listed here will "bind to vhost".
             * (I don't know for sure what this means -- it's a magic spell for now)
             *
             */
            lws_protocol_vhost_options pvo_;
#endif

            /* protocols to accept for this webserver */
            std::vector<lws_protocols> protocol_v_;

            /* mount point for dynamic urls
             * (these will be served by executing c++ code,
             *  instead of serving static disk files)
             */
            lws_http_mount mount_dynamic_;
            /* mount point for static urls
             * (serve static files from file system)
             */
            lws_http_mount mount_static_;

            /* retry settings
             * (not sure how these are used)
             */
            lws_retry_bo_t retry_;

            /* configuration record for lws context
             * AFAIK require lifetime >= lws_context
             */
            lws_context_creation_info cx_config_;

            /* protects .lws_cx -- the POINTER, against the service thread
             * publishing it at start and destroying it at stop.  See
             * .wake_service_thread()
             */
            std::mutex cx_mutex_;

            /* runtime state owned by LWS library
             * can get application-determined user data from a lws_context by
             *   lws_context_user(.lws_cx)
             *
             * Written only by the service thread, under .cx_mutex; null
             * before start and once stop begins.  Other threads use it only
             * through .wake_service_thread().
             */
            lws_context * lws_cx_ = nullptr;

            /* --- 2. startup/shutdown control --- */

            /* set this to true to prevent further service loop iteration */
            std::atomic<bool> interrupt_flag_;

            /* port the vhost is listening on; 0 while not listening.
             * Written by the service thread, read from any
             */
            std::atomic<std::int32_t> listen_port_{0};

            /* protects .state */
            std::mutex mutex_;
            std::condition_variable cond_;

            /* valid states
             *
             *   .state           .thread_ptr
             *   -----------------------------------------------
             *   running          thread in WebserverImpl::run()
             *   stop_requested   thread in WebserverImpl::run()
             *   stopped          nullptr
             */
            Runstate state_;
            std::unique_ptr<std::thread> thread_ptr_;

            /* --- 3. plugin state (writable while server runs) --- */

            /* registered http and stream endpoints, and the matching that
             * picks one for an incoming uri.  Locks internally.
             *
             * Each session's router borrows this: declared BEFORE .session_v,
             * so sessions are destroyed first.
             *
             * use .register_http_endpoint(), .register_stream_endpoint()
             * to insert
             */
            UrlRouter url_router_;

            /* protects .removed_endpoint_v */
            std::mutex removed_mutex_;
            /* stream endpoints unregistered (so already out of .url_router)
             * whose live subscriptions the service thread has yet to end.
             * Filled by .unregister_stream_endpoint() on any thread; drained
             * by .lws_end_removed_subscriptions() on the service thread.
             * Holding them keeps each alive until its subscriptions have
             * unsubscribed from it.
             */
            std::vector<rp<DynamicEndpoint>> removed_endpoint_v_;

            /* --- 4. libwebsocket session manager --- */

            /* websocket-associated libwebsocket data.
             * created by libwebsocket;  our appl code informed via
             * LWS_CALLBACK_PROTOCOL_INIT
             *
             * list of all active sessions is in .ws_vhd->pss_list
             * (see LWS_CALLBACK_PROTOCOL_INIT, LWS_CALLBACK_ESTABLISHED, LWS_CALLBACK_CLOSED)
             *
             * can visit sessions with macros:
             *   lws_start_foreach_llp(struct per_session_data__minimal **, ppss, vhd->pss_list) {
             *      ..do stuff with (*ppss)->wsi for example..
             *   } lws_end_foreach_llp(ppss, pss_list);
             */
            per_vhost_data__minimal * ws_vhd_ = nullptr;

            /* live websocket sessions, by id.  Ids are assigned at
             * LWS_CALLBACK_HTTP_BIND_PROTOCOL and never reused; a session's
             * record is removed when it closes.  Locks internally.
             */
            WsSessionTable<WebsocketSessionRecd> session_table_;

        }; /*WebserverImpl*/

        void
        WebserverImpl::register_http_endpoint(HttpEndpointDescr const & endpoint_descr)
        {
            this->url_router_.register_http(endpoint_descr);
        } /*register_http_endpoint*/

        void
        WebserverImpl::register_stream_endpoint(StreamEndpointDescr const & endpoint_descr)
        {
            this->url_router_.register_stream(endpoint_descr);
        } /*register_stream_endpoint*/

        void
        WebserverImpl::visit_sessions(SessionVisitor const & fn) const
        {
            /* lock order: session table, then (inside fn, when printing
             * subscriptions) each router's own lock.  Nothing takes them the
             * other way round: a router never calls out holding its lock
             */
            this->session_table_.for_each_by_id([&fn](std::uint64_t, WebsocketSessionRecd const & recd)
                {
                    fn(TaggedPtr(Reflect::require<WebsocketSessionRecd>(),
                                 const_cast<WebsocketSessionRecd *>(&recd)));
                });
        } /*visit_sessions*/

        namespace {
            /** @brief a session's sender: owned (held) by the session record,
             *  its router, and every sink made there.  Printed in full under
             *  its session; a sink will show it as a ref (5d)
             **/
            class JsonPrinter_WsSessionSender : public JsonPrinter {
            public:
                JsonPrinter_WsSessionSender(PrintJson const * pjson) : JsonPrinter(pjson) {}

                void print_json(TaggedPtr tp, std::ostream * p_os) const override {
                    WsSessionSenderImpl const * x
                        = this->check_recover_native<WsSessionSenderImpl>(tp, p_os);

                    if (!x)
                        return;

                    *p_os << "{" << quot("_name_") << ": " << quot("WsSessionSender")
                          << ", " << quot("id") << ": " << quot(json_id(x))
                          /* session record + router + one per live sink */
                          << ", " << quot("refcount") << ": " << x->reference_counter()
                          << ", " << quot("session_id") << ": " << x->session_id()
                          << ", " << quot("open") << ": " << (x->is_open() ? "true" : "false")
                          << "}";
                }
            };

            /** @brief a live session: its id, its sender (in full), and its
             *  subscriptions (in full, each with its sink)
             **/
            class JsonPrinter_WsSession : public JsonPrinter {
            public:
                JsonPrinter_WsSession(PrintJson const * pjson) : JsonPrinter(pjson) {}

                void print_json(TaggedPtr tp, std::ostream * p_os) const override {
                    WebsocketSessionRecd const * recd
                        = this->check_recover_native<WebsocketSessionRecd>(tp, p_os);

                    if (!recd)
                        return;

                    *p_os << "{" << quot("_name_") << ": " << quot("WsSession")
                          << ", " << quot("id") << ": " << quot(json_id(recd))
                          << ", " << quot("session_id") << ": " << recd->session_id()
                          << ", " << quot("sender") << ": ";

                    this->pjson()->print_aux(TaggedPtr(Reflect::require<WsSessionSenderImpl>(),
                                                       const_cast<WsSessionSenderImpl *>(&recd->sender())),
                                             p_os);

                    /* each subscription via its printer (in
                     * WsSessionRouter.cpp), under the router's lock
                     */
                    *p_os << ", " << quot("subscriptions") << ": [";
                    {
                        bool first = true;
                        PrintJson const * pjson = this->pjson();

                        recd->router().visit_subscriptions([pjson, p_os, &first](TaggedPtr sub) {
                                if (!first)
                                    *p_os << ", ";
                                first = false;

                                pjson->print_aux(sub, p_os);
                            });
                    }
                    *p_os << "]}";
                }
            };
        } /*namespace*/

        void
        provide_webserver_json_printers(PrintJson * pjson)
        {
            pjson->provide_printer(Reflect::require<WebsocketSessionRecd>(),
                                   std::make_unique<JsonPrinter_WsSession>(pjson));
            pjson->provide_printer(Reflect::require<WsSessionSenderImpl>(),
                                   std::make_unique<JsonPrinter_WsSessionSender>(pjson));
        } /*provide_webserver_json_printers*/

        bool
        WebserverImpl::unregister_http_endpoint(std::string const & uri_pattern)
        {
            /* http has no long-lived state to end: gone for the next request */
            return static_cast<bool>(this->url_router_.unregister_http(uri_pattern));
        } /*unregister_http_endpoint*/

        bool
        WebserverImpl::unregister_stream_endpoint(std::string const & uri_pattern)
        {
            /* 1. out of the router at once: new subscribes fail from here */
            rp<DynamicEndpoint> removed = this->url_router_.unregister_stream(uri_pattern);

            if (!removed)
                return false;

            /* 2. its live subscriptions belong to the service thread (their
             *    sinks may be receiving from a source right now): queue the
             *    ending of them there, and wake it -- as send_text does
             */
            {
                std::lock_guard<std::mutex> lock(this->removed_mutex_);

                this->removed_endpoint_v_.push_back(std::move(removed));
            }

            this->wake_service_thread();

            /* if the server is not running there are no sessions, so nothing
             * to end; the queue drains on the next wakeup, or with the server
             */
            return true;
        } /*unregister_stream_endpoint*/

#ifdef DEFINED_BUT_NOT_USED
        void
        WebserverImpl::notify_vhd(per_vhost_data__minimal * vhd)
        {
            this->ws_vhd_ = vhd;
        } /*notify_vhd*/
#endif

        void
        WebserverImpl::notify_ws_session_open(OutputBuffer * output_buf,
                                              per_vhost_data__minimal * /*vhd*/,
                                              WsSafetyToken const & ws_safety_token)
        {
            ws_safety_token.verify();

            uint64_t new_id = output_buf->session_id();

            this->session_table_.insert
                (new_id,
                 std::unique_ptr<WebsocketSessionRecd>
                 (new WebsocketSessionRecd
                  (output_buf,
                   /* endpoints serving stream names */
                   this->url_router_,
                   /* sends to THIS session: replies, and every sink's frames */
                   rp<WsSessionSenderImpl>(new WsSessionSenderImpl(this, new_id)),
                   this->pjson_)));

            /* control comes here when a new websocket session is created,
             * after LWS_CALLBACK_HTTP_BIND_PROTOCOL
             */
            output_buf->set_is_writeable(true, ws_safety_token);
        } /*notify_ws_session_open*/

        void
        WebserverImpl::notify_ws_session_close(OutputBuffer * output_buf,
                                               per_vhost_data__minimal * /*vhd*/,
                                               WsSafetyToken const & ws_safety_token)
        {
            scope log(XO_ENTER0_(info));

            log && log("enter",
                       xtag("this", (void*)this),
                       xtag("output_buf", (void*)output_buf));

            ws_safety_token.verify();

            /* out of the table first: from here the id addresses nothing,
             * ever -- ids are never reused.  Then, with the table's lock
             * released, since an endpoint's unsubscribe may send:
             */
            std::unique_ptr<WebsocketSessionRecd> ws_session_recd
                = this->session_table_.take(output_buf->session_id());

            if (ws_session_recd) {
                /* anything sent from here on, by this session's router or by a
                 * sink the application kept, is dropped
                 */
                ws_session_recd->close_sender();
                ws_session_recd->unsubscribe_all();
            }

            /* record destroyed here */
        } /*notify_ws_session_close*/

        /* note: to access lws_protocols.user,
         *       would use lws_get_protocol(wsi)->user
         */
        int
        WebserverImpl::notify_dynamic_http(struct lws * wsi,
                                           lws_callback_reasons reason,
                                           void * user_data,
                                           void * incoming_data,
                                           size_t len)
        {
            lws_context * lws_cx = lws_get_context(wsi);
            void * cx_user_data = lws_context_user(lws_cx);
            WebserverImpl * websrv = reinterpret_cast<WebserverImpl *>(cx_user_data);

            struct per_session_data__http * http_pss
                = reinterpret_cast<struct per_session_data__http *>(user_data);

            lwsl_user("notify_dynamic_http: enter: reason %d (%s): lws_cx %p websrv %p\n",
                      reason,
                      WebsockUtil::ws_callback_reason_descr(reason),
                      lws_cx,
                      websrv);

            /* scratch space for http header
             * (probably only need LWS_PRE here,  I think the +256 debris
             *  from o.g. example)
             */
            uint8_t buf[LWS_PRE + 256];
            uint8_t * start = &buf[LWS_PRE];
            uint8_t * p = start;
            uint8_t * end = &buf[sizeof(buf) - 1];

            switch (reason) {
            case LWS_CALLBACK_HTTP:
            {
                /* incoming_uri contains the uri suffix following our mountpoint [/dyn]
                 * (see WebserverImpl.init_mount_dynamic()).
                 *
                 * looks like this gets spuriously invoked for non-dynamic mountpoints
                 * given that we serve both filesystem tree
                 * (in url-space at /, from dir ./mount-origin) and dynamic http (in url-space at /dyn);
                 *
                 * however output from the spurious invocation seems to be discarded
                 */
                char const * incoming_uri
                    = reinterpret_cast<char const *>(incoming_data);

                assert(http_pss->output_str == nullptr);

                if (http_pss->output_str == nullptr) {
                    http_pss->output_str = new std::string;
                }

                lwsl_user("allocate output_str [%p] in http_pss [%p]",
                          http_pss->output_str, http_pss);

                std::stringstream response_ss;

                assert(websrv);

                websrv->dynamic_http_response(incoming_uri,
                                              &response_ss);

                *(http_pss->output_str) = response_ss.str();

                lwsl_user("LWS_CALLBACK_HTTP: got response [%s]",
                          http_pss->output_str->c_str());

                /* choose mime type */
                constexpr char const * c_mime_type = "application/json";

                /* prepare and write http headers
                 * (do these precede &p ??)
                 */
                if (lws_add_http_common_headers(wsi,
                                                HTTP_STATUS_OK,
                                                c_mime_type,
                                                http_pss->output_str->length(),
                                                &p, end))
                    return 1;

                if (lws_finalize_write_http_header(wsi, start, &p, end))
                    return 1;

                /* write the body separately */
                lws_callback_on_writable(wsi);

                return 0;
            }

            case LWS_CALLBACK_HTTP_WRITEABLE:
            {
                if (!http_pss || !http_pss->output_str || (http_pss->output_str->length() == 0))
                    break;

                /*
                 * Use LWS_WRITE_HTTP (instead of LWS_WRITE_HTTP_FINAL) for intermediate writes,
                 * on http/2 lws uses this to understand to end the stream with this
                 * frame.
                 *
                 * TODO: if output is large,  write it in smaller chunks.
                 *       expecting mtu like 1500 bytes,   so maybe 128k
                 *       chunks will work well?
                 */
                if (lws_write(wsi,
                              (uint8_t *)(http_pss->output_str->c_str()),
                              http_pss->output_str->length(),
                              LWS_WRITE_HTTP_FINAL)
                    != static_cast<int>(http_pss->output_str->length()))
                {
                    return 1;
                }

                /*
                 * HTTP/1.0 no keepalive: close network connection
                 * HTTP/1.1 or HTTP1.0 + KA: wait / process next transaction
                 * HTTP/2: stream ended, parent connection remains up
                 */
                if (lws_http_transaction_completed(wsi))
                    return -1;

                return 0;
            }

            case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
            {
                /* from libwebsocket docs:
                 *   By default, all HTTP handling is done in protocols[0].
                 *   However you can bind different protocols (by name) to different parts of the URL space using callback mounts.
                 *   This callback occurs in the new protocol when a wsi is bound to that protocol.
                 *   Any protocol allocation related to the http transaction processing should be created then.
                 *   These specific callbacks are necessary because with HTTP/1.1,
                 *   a single connection may perform a series of different transactions at different URLs,
                 *   thus the lifetime of the protocol bind is just for one transaction, not connection.
                 */
                if (!http_pss)
                    break;

                /* although we could allocate http_pss->output_ss here,
                 * instead delay until LWS_CALLBACK_HTTP.
                 * this reduces new/delete churn,  since BIND/DROP callbacks
                 * will get invoked on every incoming request,   not just for
                 * dynamic http requests.
                 */
                http_pss->output_str = nullptr;

                lwsl_user("initialize http_pss->output_str to null in http_pss [%p]",
                          http_pss);
            }
            break;

            case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
                /* from libwebsocket docs:
                 *   This is called when a transaction is unbound from a protocol.
                 *   It indicates the connection completed its transaction and may do something different now.
                 *   Any protocol allocation related to the http transaction processing should be destroyed.
                 */
                if (!http_pss)
                    break;

                if (http_pss->output_str) {
                    lwsl_user("destroy string [%p] in http_pss [%p]",
                              http_pss->output_str, http_pss);

                    delete http_pss->output_str;

                    http_pss->output_str = nullptr; /*hygiene*/
                }

            default:
                break;
            }

            return lws_callback_http_dummy(wsi, reason, user_data, incoming_data, len);

        } /*notify_dynamic_http*/

        void
        WebserverImpl::start_webserver()
        {
            switch(state_) {
            case Runstate::stopped:
            {
                std::unique_lock<std::mutex> lock(this->mutex_);

                this->thread_ptr_.reset(new std::thread(&WebserverImpl::run, this));
                this->state_ = Runstate::running;
            }
            break;
            case Runstate::stop_requested:
                throw std::runtime_error("webserver in stop-requested state");
                /* could invent a "restart-requested" state, I suppose */
                break;
            case Runstate::running:
                throw std::runtime_error("webserver already running");
                break;
            }
        } /*start_webserver*/

        void
        WebserverImpl::dynamic_http_response(std::string const & incoming_uri,
                                             std::ostream * p_os)
        {
            rp<DynamicEndpoint> endpoint = this->url_router_.find_http(incoming_uri);

            if (endpoint) {
                endpoint->http_response(incoming_uri, p_os);
                return;
            } else {
                /* if control here,  no match */

                /* or replace pss->str, pss->len with whatever dynamic content you like */
                time_t t0 = ::time(nullptr);

                *p_os << ("<html>"
                          "<img src=\"/libwebsockets.org-logo.svg\">"
                          "<br>no dynamic content for uri [")
                      << incoming_uri
                      << ("]"
                          " from mountpoint."
                          "<br>time: ")
                      << ctime(&t0)
                      << "</html>";
            }
        } /*dynamic_http_response*/

        void
        WebserverImpl::perform_ws_cmd(uint64_t session_id,
                                      std::string_view incoming_cmd)
        {
            /* parsing, subscribe and send all live in WsSessionRouter,
             * which knows nothing of libwebsockets -- see
             * .xo-backlog/xo-websock/issues/04
             *
             * table lock NOT held while the command runs: handlers send.
             * Safe: we are on the service thread, the only one that removes
             * sessions.
             */
            WebsocketSessionRecd * ws_recd
                = this->session_table_.find_owner_thread(session_id);

            assert(ws_recd);

            if (ws_recd)
                ws_recd->router().perform_cmd(incoming_cmd);
        } /*perform_ws_cmd*/

        void
        WebserverImpl::interrupt_stop_webserver()
        {
            /* NOTE: this is threadsafe - ::lws_cancel_service()
             *       writes to a pipe to interrupt polling loop
             */
            /* flag first: if the context is not yet published, the wakeup
             * below does nothing, but run() checks the flag before its first
             * lws_service() call
             */
            this->interrupt_flag_ = true;
            this->wake_service_thread();

            std::unique_lock<std::mutex> lock(this->mutex_);

            this->state_ = Runstate::stop_requested;
        } /*interrupt_stop_webserver*/

        void
        WebserverImpl::wake_service_thread()
        {
            /* the lock spans the null check AND the call, so the context
             * cannot be destroyed in between: run() nulls .lws_cx under this
             * lock before destroying it.  lws_cancel_service() writes to a
             * pipe -- never blocks, never calls back -- so holding the lock
             * across it is safe.
             */
            std::lock_guard<std::mutex> lock(this->cx_mutex_);

            if (this->lws_cx_)
                ::lws_cancel_service(this->lws_cx_);
        } /*wake_service_thread*/

        void
        WebserverImpl::stop_webserver()
        {
            bool running = false;

            {
                std::unique_lock<std::mutex> lock(this->mutex_);

                running = (this->state_ == Runstate::running);
            }

            /* lock released first: interrupt_stop_webserver() takes it, and
             * it is not recursive.  Holding it here self-deadlocked every
             * stop of a running server (found by utest.websock.live,
             * .xo-backlog/xo-websock/issues/09)
             */
            if (running)
                this->interrupt_stop_webserver();
        } /*stop_webserver*/

        void
        WebserverImpl::join_webserver() {
            while(true) {
                std::unique_lock<std::mutex> lock(this->mutex_);

                if (this->state_ == Runstate::stopped)
                    break;

                this->cond_.wait(lock);
            }

            if (this->thread_ptr_) {
                this->thread_ptr_->join();
                this->thread_ptr_ = nullptr;
            }
        } /*join_webserver*/

        void
        WebserverImpl::send_text(uint64_t session_id,
                                 std::string text)
        {
            scope log(XO_ENTER0_(info));
            log && log(xtag("session_id", session_id));

            /* any thread.  Table lock held across the record's send_text,
             * which only queues and wakes the service thread
             */
            bool live = this->session_table_.with_session
                            (session_id,
                             [&text](WebsocketSessionRecd & recd)
                                 {
                                     recd.send_text(std::move(text));
                                 });

            if (!live) {
                /* session closed: ids are never reused, so this is a stale
                 * sender, not a different client.  Dropped.
                 */
                log && log("no live session; dropped");
            }
        } /*send_text*/

        void
        WebserverImpl::lws_write_pending_traffic(WsSafetyToken const & ws_safety_token)
        {
            scope log(XO_ENTER0_(info));

            ws_safety_token.verify();

            this->session_table_.for_each([&ws_safety_token](WebsocketSessionRecd & recd)
                {
                    recd.lws_write_pending(ws_safety_token);
                });
        } /*lws_write_pending_traffic*/

        void
        WebserverImpl::lws_end_removed_subscriptions(WsSafetyToken const & ws_safety_token)
        {
            ws_safety_token.verify();

            std::vector<rp<DynamicEndpoint>> removed_v;

            {
                std::lock_guard<std::mutex> lock(this->removed_mutex_);

                removed_v.swap(this->removed_endpoint_v_);
            }

            if (removed_v.empty())
                return;

            scope log(XO_ENTER0_(info), xtag("n_removed", removed_v.size()));

            /* collect the sessions' routers under the table lock, then use
             * them with it RELEASED: end_subscriptions_on() replies through
             * the session's sender -> send_text() -> the table lock again,
             * which is not recursive.  Safe to hold the pointers: this is the
             * service thread, the only one that removes sessions.
             */
            std::vector<WsSessionRouter *> router_v;

            this->session_table_.for_each([&router_v](WebsocketSessionRecd & recd)
                {
                    router_v.push_back(&recd.router());
                });

            for (auto const & endpoint : removed_v) {
                for (WsSessionRouter * router : router_v)
                    router->end_subscriptions_on(endpoint);
            }
        } /*lws_end_removed_subscriptions*/

        /* sequester .ws_safety_token:
         * it may only be used by dedicated websocket library thread
         * (the unique thread that calls ::lws_service())
         */
        class WebserverImplWsThread : public WebserverImpl {
        public:
            WebserverImplWsThread(WebserverConfig const & ws_config,
                                  rp<PrintJson> const & pjson)
                : WebserverImpl(ws_config, pjson)
                {
                    scope log(XO_DEBUG_(true /*debug_flag*/),
                              xtag("self", (void*)this));

                    this->set_lws_log_level();
#if defined(LWS_WITH_PLUGINS)
                    this->init_pvo(&(this->pvo_));
#endif
                    this->init_protocols(&(this->protocol_v_));
                    this->init_mount_dynamic(&(this->mount_dynamic_));
                    this->init_mount_static(&(this->mount_dynamic_),
                                            &(this->mount_static_));
                    this->init_cx_config(&(this->cx_config_));
                } /*ctor*/

            /* create instance */
            static rp<WebserverImpl> make(WebserverConfig const & ws_config,
                                          rp<PrintJson> const & pjson);

            // ----- Inherited from WebserverImpl -----

            /* init helper */
            virtual void init_protocols(std::vector<lws_protocols> * p_v) override;

            /* run webserver.   borrows calling thread,  doesn't return
             * until webserver stopped.
             */
            virtual void run() override;

        private:
            /* callback for lws-minimal protocol (websocket) */
            static int notify_minimal(struct lws * wsi,
                                      lws_callback_reasons reason,
                                      void * user_data,
                                      void * incoming_uri,
                                      size_t len);

            WsSafetyToken const & ws_safety_token() const { return ws_safety_token_; }

        private:
            /* a function taking .ws_safety_token as an argument,
             * announces that it is being called from the libws thread,
             * i.e. reentrantly from ::lws_service()
             */
            WsSafetyToken ws_safety_token_;
        }; /*WebserverImplWsThread*/

        /* 1. anything after the host:port prefix will get handled by callback_dynamic_http
         * 2. host::port alone will upgrade to "lws-minimal" for websocket demo
         *
         * in practice p_v will be &WebserverImpl::protocol_v_
         */
        void
        WebserverImplWsThread::init_protocols(std::vector<lws_protocols> * p_v)
        {
            /* lws_protocols:
             *  .name
             *  .callback
             *  .per_session_data_size
             *  .rx_buffer_size
             *  .id                     advertised as accessible from callback,  but don't see how to use this
             *  .user                   advertised as accessible from callback,  but don't see how to make this work.
             *                          looks like libwebsocket allocates its own struct, even if .user is nonempty,
             *                          and whether or not .per_session_data_size is 0.
             *  .tx_packet_size
             */
            p_v->push_back(
                {"http",
                 &WebserverImpl::notify_dynamic_http,
                 sizeof(struct per_session_data__http),
                 0,
                 0,
                 NULL,
                 0
                });
            p_v->push_back(
                {"lws-minimal",
                 &WebserverImplWsThread::notify_minimal,
                 sizeof(struct per_session_data__minimal),
                 128,
                 0,
                 NULL,
                 0});
            /* mandatory end-of-array sentinel,  requires by lws */
#if ((LWS_LIBRARY_VERSION_MAJOR > 4) || (LWS_LIBRARY_VERSION_MAJOR == 4) && (LWS_LIBRARY_VERSION_MINOR >= 3))
            p_v->push_back(LWS_PROTOCOL_LIST_TERM);
#else
            p_v->push_back({ nullptr, nullptr, 0, 0, 0, nullptr, 0});
#endif
        } /*init_protocols*/

        /* called reentrantly from ::lws_service(),
         * to do work on behalf of the websocket protocol "lws-minimal"
         */
        int
        WebserverImplWsThread::notify_minimal(struct lws * wsi,
                                              lws_callback_reasons reason,
                                              void * user_data,
                                              void * input,
                                              size_t input_z)
        {
            scope log(XO_ENTER0_(info),
                      xtag("wsi", (void*)wsi));

            lwsl_user("WebserverImpl::notify_minimal: enter"
                      ": reason %d (%s)",
                      reason,
                      WebsockUtil::ws_callback_reason_descr(reason));

            assert(wsi);

            lws_context * lws_cx = lws_get_context(wsi);

            assert(lws_cx);
            void * cx_user_data = lws_context_user(lws_cx);

            WebserverImplWsThread * websrv = reinterpret_cast<WebserverImplWsThread *>(cx_user_data);
            assert(websrv);

            WsSafetyToken const & ws_token = websrv->ws_safety_token();

            struct per_session_data__minimal * ws_pss
                = ((struct per_session_data__minimal *)user_data);

            lwsl_user("WebserverImpl::notify_minimal: enter"
                      ": reason %d (%s): wsi [%p], ws_pss [%p], lws_cx [%p] websrv [%p]\n",
                      reason,
                      WebsockUtil::ws_callback_reason_descr(reason),
                      wsi,
                      ws_pss,
                      lws_cx,
                      websrv);

            struct per_vhost_data__minimal * vhd
                = ((struct per_vhost_data__minimal *)
                   lws_protocol_vh_priv_get(lws_get_vhost(wsi),
                                            lws_get_protocol(wsi)));
            int m;

            switch (reason) {
            case LWS_CALLBACK_PROTOCOL_INIT:
            {
                vhd = (reinterpret_cast<per_vhost_data__minimal *>
                       (lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
                                                    lws_get_protocol(wsi),
                                                    sizeof(struct per_vhost_data__minimal))));
                vhd->context = lws_get_context(wsi);
                vhd->vhost = lws_get_vhost(wsi);
                vhd->protocol = lws_get_protocol(wsi);
                vhd->pss_list = nullptr;
                //vhd->current = 0;

                lwsl_user("WebserverImpl::notify_minimal: vhost=%p, protocols=%p protocol.name=%s\n",
                          vhd->vhost, vhd->protocol, vhd->protocol->name);
            }
            break;

            case LWS_CALLBACK_WS_SERVER_BIND_PROTOCOL:
            case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
            {
                /* looks like control comes here with
                 *   LWS_CALLBACK_HTTP_BIND_PROTOCOL,
                 * although based on docs would seem to expect
                 *   LWS_CALLBACK_WS_SERVER_BIND_PROTOCOL
                 *
                 * In any case,  control here when new websocket session created
                 */

                if (!ws_pss)
                    break;

                assert(vhd);

                /* the id's only assignment; never reused */
                ws_pss->output_buf_ = new OutputBuffer(websrv->next_session_id());

                lwsl_user("establish pss->output_buf [%p] in ws_pss [%p]",
                          ws_pss->output_buf_,
                          ws_pss);
            }
            break;
            case LWS_CALLBACK_WS_SERVER_DROP_PROTOCOL:
            {
                if (!ws_pss)
                    break;

                lwsl_user("destroy pss->output_msg [%p] in ws_pss [%p]",
                          ws_pss->output_buf_, ws_pss);

                /* don't do this here.   need to access ws_pss->output_buf
                 * from LWS_CALLBACK_CLOSED
                 */
#ifdef BROKEN
                if (ws_pss->output_buf_) {
                    delete ws_pss->output_buf_;
                    ws_pss->output_buf_ = nullptr;
                }
#endif
            }
            break;
            case LWS_CALLBACK_ESTABLISHED:
            {
                /* control comes here when a websocket session is opened
                 * (after protocol negotiated)
                 */

                OutputBuffer * output_buf = ws_pss->output_buf_;

                output_buf->establish_wsi(wsi);

                websrv->notify_ws_session_open(output_buf, vhd, ws_token);
            }
            break;

            case LWS_CALLBACK_CLOSED:
            {
                /* control comes here when a websocket session is closed */
                assert(websrv);
                assert(ws_pss);
                assert(ws_pss->output_buf_);
                assert(vhd);

                websrv->notify_ws_session_close(ws_pss->output_buf_, vhd, ws_token);

                if (ws_pss->output_buf_) {
                    delete ws_pss->output_buf_;
                    ws_pss->output_buf_ = nullptr;
                }

                lwsl_user("LWS_CALLBACK_CLOSED: done");
            }
            break;

            case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
            {
                if (websrv) {
                    websrv->lws_end_removed_subscriptions(ws_token);
                    websrv->lws_write_pending_traffic(ws_token);
                }
            }
            break;

            case LWS_CALLBACK_SERVER_WRITEABLE:
            {
                /* control here when:
                 * 1. application wants to send data
                 *    (app uses lws_cancel_service() to trigger
                 *     LWS_CALLBACK_EVENT_WAIT_CANCELLED)
                 * 2. websocket session that was previously blocked
                 *    is now ready to receive data
                 *    (see LWS_CALLBACK_SERVER_WRITEABLE above)
                 */

#ifdef NOT_USING
                if (!vhd->amsg.payload)
                    break;
#endif

                if (!vhd) {
                    lwsl_user("client entry: vhd not yet established, return");
                    break;
                }

                if (!ws_pss) {
                    lwsl_user("client entry: ws_pss not yet established, return");
                    break;
                }

                if (!(ws_pss->output_buf_)) {
                    lwsl_user("client entry: output_msg buffer not established, return");
                    /* output message container hasn't been established,
                     * probably bc nothing to send
                     */
                    break;
                }

                lwsl_user("notify_minimal: unblock writing, output_msg=[%p]",
                          ws_pss->output_buf_);

                if (ws_pss->output_buf_->is_writeable(ws_token)) {
                    //assert(false);
                } else {
                    /* a previous call to lws_write() reported a partial write;
                     * that write has now completed
                     */
                    ws_pss->output_buf_->lws_write_completion(ws_token);
                    break;
                }

                if (ws_pss->output_buf_->is_idle()) {
                    lwsl_user("client entry: output_msg buffer up-to-date, return");
                    /* already up-to-date,  nothing new to send */
                    break;
                }

#ifdef NOT_USING
                if (ws_pss->last == vhd->current) {
                    /* already up-to-date */
                    break;
                }

                if (!pss->output_msg_.payload) {
                    pss->output_msg_.payload = ::malloc(LWS_PRE + output_z);
                    pss->output_msg_.len = output_z;
                }

                ::memcpy((char *)pss->output_msg_.payload + LWS_PRE, output_cstr, output_z);

                lwsl_user("allocate pss->output_msg [%p] in pss [%p]",
                          pss->output_msg_.payload, pss);
                m = lws_write(wsi,
                              ((unsigned char *)pss->output_msg_.payload) + LWS_PRE,
                              pss->output_msg_.len,
                              LWS_WRITE_TEXT);
#endif

                //XO_SCOPE_(lscope);

                /* pss->output_msg_ was populated from WebserverImpl.send_text(), q.v. */

                m = ws_pss->output_buf_->lws_write_aux(ws_token);

#ifdef NOT_USING
                m = lws_write(wsi, ((unsigned char *)vhd->amsg.payload) +
                              LWS_PRE, vhd->amsg.len, LWS_WRITE_TEXT);
                if (m < (int)vhd->amsg.len) { .. }
#endif
                if (m == -1) {
                    lwsl_err("WebserverImplWsThread::notify_minimal: return -1 from callback");
                    return -1;
                }

                //pss->last = vhd->current;
            }
            break;

            case LWS_CALLBACK_RECEIVE:
            {
                /* NB each callback is treated as one complete message.
                 * libwebsockets may deliver a large message in fragments
                 * (see lws_is_final_fragment()), which this does not
                 * reassemble.  Commands are small enough to arrive whole;
                 * revisit if that stops being true.
                 */
                char const * incoming_cmd
                    = reinterpret_cast<char const *>(input);

                std::string_view incoming_svw(incoming_cmd, input_z);

                //lwsl_user("receive: [%s], z [%d]", incoming_cmd, (int)input_z);

                assert(ws_pss);
                assert(websrv);

                uint64_t session_id = ws_pss->output_buf_->session_id();

                websrv->perform_ws_cmd(session_id,
                                       incoming_svw);

#ifdef OBSOLETE
                if (vhd->amsg.payload)
                    minimal_destroy_message(&(vhd->amsg));

                vhd->amsg.len = input_z; //output_z;
                /* notice we over-allocate by LWS_PRE */
                vhd->amsg.payload = ::malloc(LWS_PRE + input_z);
                if (!vhd->amsg.payload) {
                    lwsl_user("OOM: dropping\n");
                    break;
                }

                ::memcpy((char *)vhd->amsg.payload + LWS_PRE, input, input_z);
                //vhd->current++;
#endif

#ifdef OBSOLETE
                /*
                 * let everybody know we want to write something on them
                 * as soon as they are ready
                 */
                lws_start_foreach_llp(struct per_session_data__minimal **,
                                      ppss, vhd->pss_list) {
                    lws_callback_on_writable((*ppss)->wsi);
                } lws_end_foreach_llp(ppss, pss_list);
#endif
            }
            break;

            default:
                break;
            }

            log.end_scope();

            return 0;
        } /*notify_minimal*/

        void
        WebserverImplWsThread::run()
        {
            scope log(XO_DEBUG_(false /*debug_flag*/));

            lwsl_user("LWS minimal http server dynamic"
                      " | visit http://localhost:%d\n", this->ws_config_.port());
#if defined(LWS_WITH_PLUGINS)
            lwsl_user("LWS_WITH_PLUGINS present");
#endif
#if defined(LWS_WITH_TLS)
            lwsl_user("LWS_WITH_TLS present");
#endif

            /* exit when .state is stop_requested,  setting state to .stopped */

            /* service thread owns the context; other threads reach it only
             * through wake_service_thread(), via .lws_cx under .cx_mutex
             */
            lws_context * lws_cx = lws_create_context(&(this->cx_config_));

            if (!lws_cx) {
                lwsl_err("lws init failed\n");

                /* still report stopped: join_webserver() waits for exactly
                 * this, and would otherwise wait forever
                 */
                std::unique_lock<std::mutex> lock(this->mutex_);

                this->state_ = Runstate::stopped;
                this->cond_.notify_all();

                return;
            }

            {
                std::lock_guard<std::mutex> lock(this->cx_mutex_);

                this->lws_cx_ = lws_cx;
            }

            /* listening from here.  The port the OS picked, when configured
             * with port 0 -- see listen_port()
             */
            {
                lws_vhost * vhost = ::lws_get_vhost_by_name(lws_cx,
                                                            this->cx_config_.vhost_name);

                if (vhost)
                    this->listen_port_.store(::lws_get_vhost_listen_port(vhost));
            }

            std::int32_t n_event = 0;
            while ((n_event >= 0) && !(this->interrupt_flag_)) {
                n_event = ::lws_service(lws_cx,
                                        0 /*ignored (used to be timeout)*/);
            }

            log && log("webserver runner returned - service loop exited",
                       xtag("n_event", n_event),
                       xtag("interrupted", this->interrupt_flag_.load()));

            this->listen_port_.store(0);

            /* unpublish BEFORE destroying: a concurrent wake_service_thread()
             * either finished with the live context, or now sees null
             */
            {
                std::lock_guard<std::mutex> lock(this->cx_mutex_);

                this->lws_cx_ = nullptr;
            }

            lws_context_destroy(lws_cx);

            {
                std::unique_lock<std::mutex> lock(this->mutex_);

                this->state_ = Runstate::stopped;
                this->cond_.notify_all();
            }

            log && log("exit");
        } /*run*/

        rp<WebserverImpl>
        WebserverImplWsThread::make(WebserverConfig const & ws_config,
                                    rp<PrintJson> const & pjson)
        {
            return new WebserverImplWsThread(ws_config, pjson);
        } /*make*/

        // ----- Webserver -----

        rp<Webserver>
        Webserver::make(WebsockAppcx const & cx,
                        WebserverConfig const & ws_config) {
            /* cx installed the printers when it was established */
            return WebserverImplWsThread::make(ws_config, cx.print_json());
        } /*make*/

        void
        Webserver::pretty(PpSink & pp) const {
            using xo::pp::field;

            pp.pretty_struct("Webserver",
                             field("state", this->state()));
        } /*display*/

        std::string
        Webserver::display_string() const {
            using xo::pp::tostr;

            Webserver * self = const_cast<Webserver*>(this);

            return tostr(rp<Webserver>(self));
        }
    } /*namespace web*/

    namespace pp {
        XO_PRETTIFIER_VIA_CONVERSION(xo::web::Runstate, xo::web::RunstateUtil::runstate_descr)
    }
} /*namespace xo*/

/* end Webserver.cpp */
