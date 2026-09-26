/** @file reactor2websock.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  The adapter between xo-reactor and xo-websock -- see
 *  .xo-backlog/xo-websock/issues/02.
 *
 *  No webserver is started.  The websocket sink here is a recording fake,
 *  which is enough: what this subsystem owns is the WIRING between a reactor
 *  source/store and the websocket side, not the transport.
 *
 *  Expectations are OBSERVED, never predicted.
 **/

#include "xo/reactor2websock/ReactorWebsocketSink.hpp"
#include "xo/reactor2websock/reactor_endpoints.hpp"
#include <xo/reactor/AbstractSource.hpp>
#include <xo/reactor/EventStore.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/TaggedPtr.hpp>
#include <xo/webutil/Alist.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <vector>

namespace xo {
    using xo::web::WebsocketSink;
    using xo::web::ReactorWebsocketSink;
    using xo::web::Alist;
    using xo::reactor::AbstractSink;
    using xo::reactor::AbstractSource;
    using xo::reactor::AbstractEventStore;
    using xo::reactor::AbstractEventProcessor;
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;
    using xo::json::PrintJson;
    using xo::fn::CallbackId;

    namespace ut {
        namespace {
            /** stands in for the webserver's per-subscription sink:
             *  records what arrives instead of sending it
             **/
            class RecordingWebsocketSink : public WebsocketSink {
            public:
                virtual std::string const & stream_name() const override { return stream_name_; }
                virtual uint32_t n_in_ev() const override { return n_in_ev_; }
                virtual void notify_ev_tp(TaggedPtr const & ev_tp) override {
                    ++n_in_ev_;
                    last_td_ = ev_tp.td();
                    last_address_ = ev_tp.address();
                }
                virtual void pretty(xo::pp::PpSink & sink) const override { sink.put("<RecordingWebsocketSink>"); }
                virtual std::string display_string() const override { return "<RecordingWebsocketSink>"; }

                std::string stream_name_ = "/ws/test";
                uint32_t n_in_ev_ = 0;
                TypeDescr last_td_ = nullptr;
                void * last_address_ = nullptr;
            };

            /** a source events can be pushed through by hand, which also
             *  reports its own destruction -- see the lifetime cases
             **/
            class TestSource : public AbstractSource {
            public:
                explicit TestSource(bool * p_destroyed = nullptr) : p_destroyed_{p_destroyed} {}
                ~TestSource() override { if (p_destroyed_) *p_destroyed_ = true; }

                // AbstractEventProcessor
                virtual std::string const & name() const override { return name_; }
                virtual void set_name(std::string const & x) override { name_ = x; }
                virtual void visit_direct_consumers(std::function<void (bp<AbstractEventProcessor>)> const &) override {}
                virtual void pretty(xo::pp::PpSink & sink) const override { sink.put("<TestSource>"); }

                // AbstractSource
                virtual TypeDescr source_ev_type() const override { return Reflect::require<int>(); }
                virtual bool is_volatile() const override { return true; }
                virtual uint32_t n_queued_out_ev() const override { return 0; }
                virtual uint32_t n_out_ev() const override { return 0; }
                virtual bool debug_sim_flag() const override { return false; }
                virtual void set_debug_sim_flag(bool) override {}
                virtual CallbackId attach_sink(rp<AbstractSink> const & sink) override {
                    CallbackId id = CallbackId::generate();
                    sinks_.push_back({id, sink});
                    return id;
                }
                virtual void detach_sink(CallbackId id) override {
                    std::erase_if(sinks_, [id](auto const & x) { return x.first == id; });
                }
                virtual std::uint64_t deliver_one() override { return 0; }

                /** deliver @p ev to every attached sink **/
                void emit(int & ev) {
                    for (auto & x : sinks_)
                        x.second->notify_ev_tp(Reflect::make_tp(&ev));
                }

                std::string name_;
                std::vector<std::pair<CallbackId, rp<AbstractSink>>> sinks_;
                bool * p_destroyed_ = nullptr;
            };

            /** a store whose snapshot is a fixed marker, and which reports
             *  its own destruction
             **/
            class TestEventStore : public AbstractEventStore {
            public:
                explicit TestEventStore(bool * p_destroyed = nullptr) : p_destroyed_{p_destroyed} {}
                ~TestEventStore() override { if (p_destroyed_) *p_destroyed_ = true; }

                virtual bool empty() const override { return true; }
                virtual std::uint32_t size() const override { return 0; }
                virtual void http_snapshot(rp<PrintJson> const &, std::ostream * p_os) const override {
                    *p_os << "SNAPSHOT";
                }
                virtual void clear() override {}
                virtual void insert_tp(TaggedPtr const &) override {}

                bool * p_destroyed_ = nullptr;
            };
        } /*namespace*/

        TEST_CASE("adapter-forwards-to-the-websocket-sink", "[reactor2websock]")
        {
            rp<RecordingWebsocketSink> rec = new RecordingWebsocketSink();
            rp<ReactorWebsocketSink> adapter = ReactorWebsocketSink::make(rec);

            /* the two properties that let ANY reactor source attach to it:
             * events of any reflected type, which need not outlive delivery
             */
            REQUIRE(adapter->allow_polymorphic_source());
            REQUIRE(adapter->allow_volatile_source());

            int ev = 42;
            adapter->notify_ev_tp(Reflect::make_tp(&ev));

            REQUIRE(rec->n_in_ev_ == 1);
            REQUIRE(rec->last_td_ == Reflect::require<int>());
            REQUIRE(rec->last_address_ == &ev);

            /* counted once, where the event is actually sent */
            REQUIRE(adapter->n_in_ev() == 1);
            REQUIRE(adapter->ws_sink().get() == rec.get());
        } /*TEST_CASE(adapter-forwards-to-the-websocket-sink)*/

        TEST_CASE("stream-endpoint-subscribes-through-the-adapter", "[reactor2websock]")
        {
            rp<TestSource> src = new TestSource();
            rp<RecordingWebsocketSink> rec = new RecordingWebsocketSink();

            auto descr = xo::web::stream_endpoint_descr(src, "/ws/test");

            REQUIRE(descr.uri_pattern() == "/ws/test");

            /* what the webserver does on {"cmd": "subscribe", ...} */
            CallbackId id = descr.subscribe_fn()(rec);

            REQUIRE(src->sinks_.size() == 1);

            auto * adapter = dynamic_cast<ReactorWebsocketSink *>(src->sinks_[0].second.get());

            REQUIRE(adapter);
            REQUIRE(adapter->ws_sink().get() == rec.get());

            int ev = 7;
            src->emit(ev);

            REQUIRE(rec->n_in_ev_ == 1);
            REQUIRE(rec->last_address_ == &ev);

            descr.unsubscribe_fn()(id);

            REQUIRE(src->sinks_.empty());
        } /*TEST_CASE(stream-endpoint-subscribes-through-the-adapter)*/

        TEST_CASE("stream-endpoint-keeps-its-source-alive", "[reactor2websock]")
        {
            /* the member version captured a raw `this', so an endpoint
             * registered with a webserver could outlive its source.  The free
             * function holds it by rp<>.
             */
            bool destroyed = false;

            {
                auto descr = [&destroyed]() {
                    rp<TestSource> src = new TestSource(&destroyed);
                    return xo::web::stream_endpoint_descr(src, "/ws/test");
                }();

                /* caller's reference is gone; the endpoint's is not */
                REQUIRE(!destroyed);
            }

            REQUIRE(destroyed);
        } /*TEST_CASE(stream-endpoint-keeps-its-source-alive)*/

        TEST_CASE("http-endpoint-serves-a-snapshot-at-snap", "[reactor2websock]")
        {
            rp<TestEventStore> store = new TestEventStore();

            /* the store here ignores its PrintJson, so none is needed */
            auto descr = xo::web::http_endpoint_descr(store, rp<PrintJson>(), "/es");

            REQUIRE(descr.uri_pattern() == "/es/snap");

            std::stringstream ss;
            descr.endpoint_fn()("/es/snap", Alist(), &ss);

            REQUIRE(ss.str() == "SNAPSHOT");
        } /*TEST_CASE(http-endpoint-serves-a-snapshot-at-snap)*/

        TEST_CASE("http-endpoint-keeps-its-store-alive", "[reactor2websock]")
        {
            bool destroyed = false;

            {
                auto descr = [&destroyed]() {
                    rp<TestEventStore> store = new TestEventStore(&destroyed);
                    return xo::web::http_endpoint_descr(store, rp<PrintJson>(), "/es");
                }();

                REQUIRE(!destroyed);

                /* and still usable -- this is what a raw `this' got wrong */
                std::stringstream ss;
                descr.endpoint_fn()("/es/snap", Alist(), &ss);
                REQUIRE(ss.str() == "SNAPSHOT");
            }

            REQUIRE(destroyed);
        } /*TEST_CASE(http-endpoint-keeps-its-store-alive)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end reactor2websock.test.cpp */
