/** @file WsSessionTable.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  WsSessionTable with a fake per-session record: the never-reused id
 *  guarantee (.xo-backlog/xo-websock/issues/08), and that a closed session
 *  is unreachable.
 *
 *  Expectations are OBSERVED, never predicted.
 **/

#include "xo/websock/WsSessionTable.hpp"
#include <catch2/catch.hpp>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace xo {
    using xo::web::WsSessionTable;

    namespace ut {
        namespace {
            /** stands in for the webserver's per-session record **/
            struct FakeRecd {
                explicit FakeRecd(std::string name) : name_{std::move(name)} {}

                std::string name_;
                std::vector<std::string> sent_v_;
            };

            using Table = WsSessionTable<FakeRecd>;
            using SessionId = Table::SessionId;

            /** open a session named @p name; its id **/
            SessionId open(Table * table, std::string name) {
                SessionId id = table->next_id();
                table->insert(id, std::make_unique<FakeRecd>(std::move(name)));
                return id;
            }
        }

        TEST_CASE("session-table-ids-are-never-reused", "[websock][WsSessionTable]")
        {
            /* the vector-and-free-list table this replaced handed a closed
             * session's id to the next connection
             */
            Table table;
            std::set<SessionId> seen;

            for (int round = 0; round < 50; ++round) {
                SessionId a = open(&table, "a");
                SessionId b = open(&table, "b");

                REQUIRE(seen.insert(a).second);
                REQUIRE(seen.insert(b).second);

                /* close in both orders, so any recycling would show */
                if (round % 2) {
                    REQUIRE(table.take(a));
                    REQUIRE(table.take(b));
                } else {
                    REQUIRE(table.take(b));
                    REQUIRE(table.take(a));
                }
            }

            REQUIRE(seen.size() == 100);
            REQUIRE(table.size() == 0);
            /* ids start at 1 and only increase */
            REQUIRE(*seen.begin() == 1);
            REQUIRE(*seen.rbegin() == 100);
        }

        TEST_CASE("session-table-a-closed-session-is-unreachable", "[websock][WsSessionTable]")
        {
            Table table;
            SessionId a = open(&table, "a");
            SessionId b = open(&table, "b");

            std::unique_ptr<FakeRecd> taken = table.take(a);

            REQUIRE(taken);
            REQUIRE(taken->name_ == "a");

            /* the id now addresses nothing, by any route */
            bool called = false;
            REQUIRE(!table.with_session(a, [&called](FakeRecd &) { called = true; }));
            REQUIRE(!called);
            REQUIRE(table.find_owner_thread(a) == nullptr);
            REQUIRE(!table.take(a));

            /* a session opened afterwards gets a new id, and a's id still
             * reaches nothing -- not the newcomer
             */
            SessionId c = open(&table, "c");

            REQUIRE(c != a);
            REQUIRE(!table.with_session(a, [](FakeRecd &) {}));

            /* the others are untouched */
            REQUIRE(table.find_owner_thread(b)->name_ == "b");
            REQUIRE(table.find_owner_thread(c)->name_ == "c");
            REQUIRE(table.size() == 2);
        }

        TEST_CASE("session-table-with-session-reaches-exactly-that-session", "[websock][WsSessionTable]")
        {
            Table table;
            SessionId a = open(&table, "a");
            SessionId b = open(&table, "b");

            REQUIRE(table.with_session(b, [](FakeRecd & r) { r.sent_v_.push_back("hello"); }));

            REQUIRE(table.find_owner_thread(a)->sent_v_.empty());
            REQUIRE(table.find_owner_thread(b)->sent_v_ == std::vector<std::string>{"hello"});

            /* an id never handed out reaches nothing either */
            REQUIRE(!table.with_session(b + 1000, [](FakeRecd &) {}));
        }

        TEST_CASE("session-table-for-each-visits-live-sessions-only", "[websock][WsSessionTable]")
        {
            Table table;
            open(&table, "a");
            SessionId b = open(&table, "b");
            open(&table, "c");

            table.take(b);

            std::set<std::string> visited;
            table.for_each([&visited](FakeRecd & r) { visited.insert(r.name_); });

            REQUIRE(visited == std::set<std::string>{"a", "c"});
        }

        TEST_CASE("session-table-const-for-each-reads-live-sessions", "[websock][WsSessionTable]")
        {
            Table table;
            open(&table, "a");
            SessionId b = open(&table, "b");
            table.take(b);

            Table const & ctable = table;
            std::set<std::string> visited;

            ctable.for_each([&visited](FakeRecd const & r) { visited.insert(r.name_); });

            REQUIRE(visited == std::set<std::string>{"a"});
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end WsSessionTable.test.cpp */
