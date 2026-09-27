/** @file WsTestClient.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "WsTestClient.hpp"
#include <cstring>
#include <vector>

namespace xo {
    namespace ut {
        namespace {
            /* the webserver's websocket protocol; see WebserverImplWsThread::init_protocols */
            constexpr char const * c_protocol = "lws-minimal";
        }

        WsTestClient::WsTestClient(std::int32_t port)
            : port_{port},
              thread_{&WsTestClient::run, this}
        {}

        WsTestClient::~WsTestClient()
        {
            lws_context * cx = nullptr;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                this->stop_ = true;
                cx = this->cx_;
            }

            if (cx)
                ::lws_cancel_service(cx);

            this->thread_.join();
        }

        bool
        WsTestClient::wait_connected(Timeout timeout)
        {
            std::unique_lock<std::mutex> lock(this->mutex_);

            return this->cv_.wait_for(lock, timeout,
                                      [this] { return this->connected_ || this->failed_; })
                && this->connected_;
        }

        void
        WsTestClient::send(std::string text)
        {
            lws_context * cx = nullptr;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                this->outbox_.push_back(std::move(text));
                cx = this->cx_;
            }

            /* wake the client thread: it asks for a writeable callback */
            if (cx)
                ::lws_cancel_service(cx);
        }

        bool
        WsTestClient::wait_received(std::size_t n, Timeout timeout)
        {
            std::unique_lock<std::mutex> lock(this->mutex_);

            return this->cv_.wait_for(lock, timeout,
                                      [this, n] { return this->received_v_.size() >= n; });
        }

        std::vector<std::string>
        WsTestClient::received() const
        {
            std::lock_guard<std::mutex> lock(this->mutex_);

            return this->received_v_;
        }

        bool
        WsTestClient::close(Timeout timeout)
        {
            lws_context * cx = nullptr;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                this->close_requested_ = true;
                cx = this->cx_;
            }

            if (cx)
                ::lws_cancel_service(cx);

            std::unique_lock<std::mutex> lock(this->mutex_);

            return this->cv_.wait_for(lock, timeout,
                                      [this] { return this->closed_ || this->failed_; })
                && this->closed_;
        }

        void
        WsTestClient::run()
        {
            lws_protocols protocol_v[2];
            std::memset(protocol_v, 0, sizeof(protocol_v));

            protocol_v[0].name = c_protocol;
            protocol_v[0].callback = &WsTestClient::callback;
            protocol_v[0].rx_buffer_size = 0;

            lws_context_creation_info cx_info;
            std::memset(&cx_info, 0, sizeof(cx_info));

            cx_info.port = CONTEXT_PORT_NO_LISTEN;
            cx_info.protocols = protocol_v;
            cx_info.user = this;

            lws_context * cx = ::lws_create_context(&cx_info);

            if (!cx) {
                std::lock_guard<std::mutex> lock(this->mutex_);

                this->failed_ = true;
                this->cv_.notify_all();
                return;
            }

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                this->cx_ = cx;
            }

            lws_client_connect_info cc_info;
            std::memset(&cc_info, 0, sizeof(cc_info));

            cc_info.context = cx;
            cc_info.address = "localhost";
            cc_info.port = this->port_;
            cc_info.path = "/";
            cc_info.host = "localhost";
            cc_info.origin = "localhost";
            cc_info.protocol = c_protocol;
            cc_info.pwsi = &(this->wsi_);

            if (!::lws_client_connect_via_info(&cc_info)) {
                std::lock_guard<std::mutex> lock(this->mutex_);

                this->failed_ = true;
                this->cv_.notify_all();
            }

            for (;;) {
                {
                    std::lock_guard<std::mutex> lock(this->mutex_);

                    if (this->stop_)
                        break;
                }

                if (::lws_service(cx, 0) < 0)
                    break;
            }

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                this->cx_ = nullptr;
            }

            ::lws_context_destroy(cx);
        } /*run*/

        int
        WsTestClient::callback(lws * wsi,
                               lws_callback_reasons reason,
                               void * /*user*/,
                               void * in,
                               std::size_t len)
        {
            WsTestClient * self
                = static_cast<WsTestClient *>(::lws_context_user(::lws_get_context(wsi)));

            if (!self)
                return 0;

            switch (reason) {
            case LWS_CALLBACK_CLIENT_ESTABLISHED:
            {
                std::lock_guard<std::mutex> lock(self->mutex_);

                self->connected_ = true;
                self->cv_.notify_all();

                if (!self->outbox_.empty())
                    ::lws_callback_on_writable(wsi);
            }
            break;

            case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            {
                std::lock_guard<std::mutex> lock(self->mutex_);

                self->failed_ = true;
                self->wsi_ = nullptr;
                self->cv_.notify_all();
            }
            break;

            case LWS_CALLBACK_CLIENT_RECEIVE:
            {
                std::lock_guard<std::mutex> lock(self->mutex_);

                self->partial_.append(static_cast<char const *>(in), len);

                if (::lws_is_final_fragment(wsi)) {
                    self->received_v_.push_back(std::move(self->partial_));
                    self->partial_.clear();
                    self->cv_.notify_all();
                }
            }
            break;

            case LWS_CALLBACK_CLIENT_WRITEABLE:
            {
                std::string text;
                bool more = false;

                {
                    std::lock_guard<std::mutex> lock(self->mutex_);

                    if (self->close_requested_)
                        return -1;   /* lws closes the connection */

                    if (self->outbox_.empty())
                        break;

                    text = std::move(self->outbox_.front());
                    self->outbox_.pop_front();
                    more = !self->outbox_.empty();
                }

                std::vector<unsigned char> buf(LWS_PRE + text.size());
                std::memcpy(buf.data() + LWS_PRE, text.data(), text.size());

                int m = ::lws_write(wsi, buf.data() + LWS_PRE, text.size(), LWS_WRITE_TEXT);

                if (m < static_cast<int>(text.size()))
                    return -1;

                if (more)
                    ::lws_callback_on_writable(wsi);
            }
            break;

            case LWS_CALLBACK_CLIENT_CLOSED:
            {
                std::lock_guard<std::mutex> lock(self->mutex_);

                self->closed_ = true;
                self->wsi_ = nullptr;
                self->cv_.notify_all();
            }
            break;

            case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
            {
                /* from send() or close(), on another thread.  Arrives on a
                 * context-level wsi, not our connection: use .wsi
                 */
                std::lock_guard<std::mutex> lock(self->mutex_);

                if (self->wsi_ && self->connected_
                    && (self->close_requested_ || !self->outbox_.empty()))
                {
                    ::lws_callback_on_writable(self->wsi_);
                }
            }
            break;

            default:
                break;
            }

            return 0;
        } /*callback*/
    } /*namespace ut*/
} /*namespace xo*/

/* end WsTestClient.cpp */
