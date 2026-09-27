/** @file WsSessionSender.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  WsSessionSender with a fake target standing in for the webserver: an
 *  open sender forwards, tagged with its session id; a closed one drops.
 *  See .xo-backlog/xo-websock/issues/05 (step C).
 *
 *  Expectations are OBSERVED, never predicted.
 **/

#include "xo/websock/WsSessionSender.hpp"
#include <catch2/catch.hpp>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace xo {
    using xo::web::WsSessionSender;
    using xo::web::WsSender;

    namespace ut {
        namespace {
            /** stands in for WebserverImpl: records every send_text **/
            struct FakeTarget {
                void send_text(std::uint64_t session_id, std::string text) {
                    sent_v_.emplace_back(session_id, std::move(text));
                }

                std::vector<std::pair<std::uint64_t, std::string>> sent_v_;
            };

            using Sender = WsSessionSender<FakeTarget>;
        }

        TEST_CASE("session-sender-forwards-with-its-session-id", "[websock][WsSessionSender]")
        {
            FakeTarget target;
            rp<Sender> a(new Sender(&target, 7));
            rp<Sender> b(new Sender(&target, 9));

            REQUIRE(a->is_open());
            REQUIRE(a->session_id() == 7);

            a->send_text("one");
            b->send_text("two");
            a->send_text("three");

            using Sent = std::vector<std::pair<std::uint64_t, std::string>>;
            REQUIRE(target.sent_v_ == Sent{{7, "one"}, {9, "two"}, {7, "three"}});
        }

        TEST_CASE("session-sender-drops-once-closed", "[websock][WsSessionSender]")
        {
            /* what keeps a sink retained past its session (or its server) from
             * reaching the server
             */
            FakeTarget target;
            rp<Sender> sender(new Sender(&target, 7));

            sender->send_text("before");
            sender->close();

            REQUIRE(!sender->is_open());

            sender->send_text("after");

            /* through the base interface, as sinks and the router hold it */
            rp<WsSender> as_base = sender;
            as_base->send_text("after, via WsSender");

            REQUIRE(target.sent_v_.size() == 1);
            REQUIRE(target.sent_v_[0].second == "before");
        }

        TEST_CASE("session-sender-close-is-idempotent", "[websock][WsSessionSender]")
        {
            FakeTarget target;
            rp<Sender> sender(new Sender(&target, 7));

            sender->close();
            sender->close();

            REQUIRE(!sender->is_open());

            sender->send_text("x");

            REQUIRE(target.sent_v_.empty());
        }

        TEST_CASE("session-sender-closed-never-touches-its-target", "[websock][WsSessionSender]")
        {
            /* the server may be freed while a sink still holds this sender:
             * once closed, the target pointer must not be followed.  A null
             * target makes any use fatal.
             */
            rp<WsSessionSender<FakeTarget>> sender(new WsSessionSender<FakeTarget>(nullptr, 7));

            sender->close();

            REQUIRE_NOTHROW(sender->send_text("into the void"));
            REQUIRE(!sender->is_open());
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end WsSessionSender.test.cpp */
