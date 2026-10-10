/** @file PrintJsonCycle.test.cpp
 *
 *  PrintJson on graphs that nest deeply or cycle:
 *  .xo-backlog/xo-printjson/issues/02.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/init_printjson.hpp"
#include "xo/printjson/JsonPrintState.hpp"
#include "xo/printjson/JsonObject.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <catch2/catch.hpp>
#include <csignal>
#include <sstream>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::StructReflector;

    namespace ut {
        namespace {
            /** a node in a singly-linked graph: a raw pointer member, so a
             *  graph of them can be a chain, or cycle.  Reflected shared --
             *  placed at first appearance -- since a raw pointer is borrowed
             *  by default, and would print only refs
             *  (.xo-backlog/xo-printjson/issues/08)
             **/
            struct Node {
                int id_;
                Node * next_;
            };

            void reflect_node() {
                static bool s_once = []() {
                    StructReflector<Node> sr;
                    sr.reflect_member("id", &Node::id_);
                    sr.reflect_member("next", &Node::next_).shared();
                    sr.require_complete();
                    return true;
                }();
                (void)s_once;
            }

            /** a node with two successors: a diamond when both are one node **/
            struct Fork {
                int id_;
                Fork * left_;
                Fork * right_;
            };

            /** an object whose first member is another object: the two
             *  share an address
             **/
            struct Inner {
                int v_;
            };

            struct Outer {
                Inner in_;
                int x_;
            };

            void reflect_more() {
                static bool s_once = []() {
                    {
                        StructReflector<Fork> sr;
                        sr.reflect_member("id", &Fork::id_);
                        /* shared: see Node */
                        sr.reflect_member("left", &Fork::left_).shared();
                        sr.reflect_member("right", &Fork::right_).shared();
                        sr.require_complete();
                    }
                    {
                        StructReflector<Inner> sr;
                        sr.reflect_member("v", &Inner::v_);
                        sr.require_complete();
                    }
                    {
                        StructReflector<Outer> sr;
                        sr.reflect_member("in", &Outer::in_);
                        sr.reflect_member("x", &Outer::x_);
                        sr.require_complete();
                    }
                    return true;
                }();
                (void)s_once;
            }

            /** an object's head as the struct printer writes it:
             *  {"_name_": .., its type keys, and "_id_": @p id (none for 0)
             **/
            template <typename T>
            std::string head(char const * name, int id) {
                std::string canonical(xo::reflect::type_name<T>());
                return "{\"_name_\": \"" + std::string(name) + "\""
                    + ", \"_canonical_type_\": \"" + canonical + "\""
                    + ", \"_short_type_\": \""
                    + xo::reflect::TypeDescrBase::make_short_name(canonical) + "\""
                    + (id ? ", \"_id_\": " + std::to_string(id) : std::string());
            }

            /** one "_members_" entry: member @p name, declared type T
             *  (metatype @p metatype), printed as @p value
             **/
            template <typename T>
            std::string entry(char const * name, char const * metatype, std::string const & value) {
                std::string canonical(xo::reflect::type_name<T>());
                return "{\"_name_\": \"" + std::string(name) + "\""
                    + ", \"_canonical_type_\": \"" + canonical + "\""
                    + ", \"_short_type_\": \""
                    + xo::reflect::TypeDescrBase::make_short_name(canonical) + "\""
                    + ", \"_metatype_\": \"" + metatype + "\""
                    + ", \"_value_\": " + value + "}";
            }

            /** a whole object: @p head (from head()), then its members **/
            std::string object(std::string const & head, std::vector<std::string> const & entries) {
                std::string retval = head + ", \"_members_\": [";
                for (std::size_t i = 0; i < entries.size(); ++i)
                    retval += (i ? ", " : "") + entries[i];
                return retval + "]}";
            }

            /** a Node, members-style **/
            std::string node(int id, int value, std::string const & next) {
                return object(head<Node>("Node", id),
                              {entry<int>("id", "atomic", std::to_string(value)),
                               entry<Node *>("next", "pointer", next)});
            }

            /** occurrences of @p pat in @p s **/
            std::size_t count(std::string const & s, std::string const & pat) {
                std::size_t n = 0;
                for (std::size_t i = s.find(pat); i != std::string::npos; i = s.find(pat, i + 1))
                    ++n;
                return n;
            }

            /** outcome of running a function in a child process **/
            struct ChildOutcome {
                /** child ended by this signal; 0 if it exited **/
                int signal_ = 0;
                /** child's exit status, if it exited **/
                int exit_status_ = 0;
                /** everything the child wrote on stderr **/
                std::string stderr_;
            };

            /** run @p fn in a forked child, its stderr captured; the child
             *  exits 0 if @p fn returns.  A death test: Catch2 has none,
             *  and ctest reports an aborting test as failed even with
             *  WILL_FAIL (cmake 3.31, measured 2026-10-04)
             **/
            template <typename Fn>
            ChildOutcome run_in_child(Fn && fn) {
                int fds[2];
                REQUIRE(::pipe(fds) == 0);

                std::cout.flush();
                std::cerr.flush();

                pid_t pid = ::fork();
                REQUIRE(pid >= 0);

                if (pid == 0) {
                    /* Catch2's own SIGABRT handler would report a failed
                     * test from the child, then re-raise
                     */
                    std::signal(SIGABRT, SIG_DFL);

                    ::close(fds[0]);
                    ::dup2(fds[1], STDERR_FILENO);
                    ::close(fds[1]);

                    fn();
                    ::_exit(0);
                }

                ::close(fds[1]);

                /* drain before waiting: the child's backtrace can outgrow
                 * the pipe's buffer
                 */
                ChildOutcome retval;
                char buf[4096];
                for (ssize_t n; (n = ::read(fds[0], buf, sizeof(buf))) > 0; )
                    retval.stderr_.append(buf, n);
                ::close(fds[0]);

                int status = 0;
                REQUIRE(::waitpid(pid, &status, 0) == pid);

                if (WIFSIGNALED(status))
                    retval.signal_ = WTERMSIG(status);
                else if (WIFEXITED(status))
                    retval.exit_status_ = WEXITSTATUS(status);

                return retval;
            }
        } /*namespace*/

        TEST_CASE("print-json-chain-within-depth-limit", "[printjson][cycle]") {
            /* depth counts JsonPrintState::print calls in progress: a node
             * nests its next_ pointer, the pointer its pointee.  So two
             * nodes and a null are four deep, within a limit of four
             */
            reflect_node();

            PrintJson print_json;
            print_json.assign_max_depth(4);

            Node b{2, nullptr};
            Node a{1, &b};

            std::stringstream ss;
            print_json.print(a, &ss);

            REQUIRE(ss.str() == node(1, 1, node(2, 2, "null")));
        } /*TEST_CASE(print-json-chain-within-depth-limit)*/

        TEST_CASE("print-json-depth-limit-aborts", "[printjson][cycle]") {
            /* the same chain, one level short of room: abort, not truncate */
            reflect_node();

            ChildOutcome x = run_in_child([]() {
                PrintJson print_json;
                print_json.assign_max_depth(3);

                Node b{2, nullptr};
                Node a{1, &b};

                std::stringstream ss;
                print_json.print(a, &ss);
            });

            INFO(x.stderr_.substr(0, 2000));
            REQUIRE(x.signal_ == SIGABRT);
            REQUIRE(x.stderr_.find("PrintJson: nesting would exceed max_depth (3)") != std::string::npos);
        } /*TEST_CASE(print-json-depth-limit-aborts)*/

        TEST_CASE("print-json-cycle", "[printjson][cycle]") {
            /* a -> b -> a: b's next is a, printed already -- a ref */
            reflect_node();

            PrintJson print_json;

            Node a{1, nullptr};
            Node b{2, &a};
            a.next_ = &b;

            std::stringstream ss;
            print_json.print(a, &ss);

            REQUIRE(ss.str() == node(1, 1, node(2, 2, "{\"_ref_\": 1}")));
        } /*TEST_CASE(print-json-cycle)*/

        TEST_CASE("print-json-self-loop", "[printjson][cycle]") {
            reflect_node();

            PrintJson print_json;

            Node a{1, nullptr};
            a.next_ = &a;

            std::stringstream ss;
            print_json.print(a, &ss);

            REQUIRE(ss.str() == node(1, 1, "{\"_ref_\": 1}"));
        } /*TEST_CASE(print-json-self-loop)*/

        TEST_CASE("print-json-diamond", "[printjson][cycle]") {
            /* a -> {d, d}: d in full under left, a ref under right */
            reflect_more();

            PrintJson print_json;

            Fork d{2, nullptr, nullptr};
            Fork a{1, &d, &d};

            std::stringstream ss;
            print_json.print(a, &ss);

            auto fork = [](int id, std::string const & left, std::string const & right) {
                return object(head<Fork>("Fork", id),
                              {entry<int>("id", "atomic", std::to_string(id)),
                               entry<Fork *>("left", "pointer", left),
                               entry<Fork *>("right", "pointer", right)});
            };

            REQUIRE(ss.str() == fork(1, fork(2, "null", "null"), "{\"_ref_\": 2}"));
        } /*TEST_CASE(print-json-diamond)*/

        TEST_CASE("print-json-diamond-chain-is-linear", "[printjson][cycle]") {
            /* 40 diamonds in a row: 2^40 paths, so printing each path in
             * full would never finish.  Each node once: 41 objects, 40 refs
             */
            reflect_more();

            constexpr int n = 40;
            std::vector<Fork> v(n + 1);
            for (int i = 0; i <= n; ++i)
                v[i] = Fork{i, i < n ? &v[i + 1] : nullptr, i < n ? &v[i + 1] : nullptr};

            PrintJson print_json;

            std::stringstream ss;
            print_json.print(v[0], &ss);

            REQUIRE(count(ss.str(), "\"_id_\"") == n + 1);
            REQUIRE(count(ss.str(), "\"_ref_\"") == n);
        } /*TEST_CASE(print-json-diamond-chain-is-linear)*/

        TEST_CASE("print-json-ref-before-print", "[printjson][cycle]") {
            /* a ref made before its object prints: the object, printing
             * later, takes the ref's id
             */
            reflect_node();

            PrintJson print_json;
            Node a{1, nullptr};

            std::stringstream ss;
            json::JsonPrintState state(&print_json, &ss);

            state.print_ref(&a);
            ss << " ";
            state.print(xo::reflect::Reflect::make_tp(&a));

            REQUIRE(ss.str() == "{\"_ref_\": 1} " + node(1, 1, "null"));
        } /*TEST_CASE(print-json-ref-before-print)*/

        TEST_CASE("print-json-first-member-shares-address", "[printjson][cycle]") {
            /* in_ is at its Outer's address: part of the Outer, so in full
             * and without an _id_ -- not a ref to the Outer
             */
            reflect_more();

            PrintJson print_json;
            Outer o{Inner{7}, 8};

            REQUIRE(static_cast<void *>(&o) == static_cast<void *>(&o.in_));

            std::stringstream ss;
            print_json.print(o, &ss);

            REQUIRE(ss.str() == object(head<Outer>("Outer", 1),
                                       {entry<Inner>("in", "struct",
                                                     object(head<Inner>("Inner", 0),
                                                            {entry<int>("v", "atomic", "7")})),
                                        entry<int>("x", "atomic", "8")}));
        } /*TEST_CASE(print-json-first-member-shares-address)*/

        namespace {
            /* never reflected: an atomic, with no printer */
            struct Opaque {
                int z_;
            };
        }

        TEST_CASE("print-json-ref-to-an-object-a-printer-wrote", "[printjson][cycle]") {
            /* an object a printer writes inline itself (open_object_at),
             * of a type with no printer and no reflection: a later pointer
             * to it is a ref, not an unprintable atomic -- refs need no
             * printer for their target's type (.xo-backlog/xo-reflect/issues/04)
             */
            PrintJson print_json;
            Opaque x{3};
            Opaque * px = &x;

            std::stringstream ss;
            json::JsonPrintState state(&print_json, &ss);

            {
                json::JsonObject obj = state.open_object_at(&x, "Opaque",
                                                            xo::reflect::Reflect::require<Opaque>());
                obj.close();
            }
            ss << " ";
            state.print(xo::reflect::Reflect::make_tp(&px));

            REQUIRE(ss.str() == head<Opaque>("Opaque", 1) + "} {\"_ref_\": 1}");
        } /*TEST_CASE(print-json-ref-to-an-object-a-printer-wrote)*/

        TEST_CASE("json-members-ref-to-an-object-a-printer-wrote", "[printjson][cycle]") {
            /* JsonMembers counts such a pointer printable: its value is a
             * ref (was: a "type not reflected" error entry)
             */
            PrintJson print_json;
            Opaque x{3};
            Opaque * px = &x;

            std::stringstream ss;
            json::JsonPrintState state(&print_json, &ss);

            {
                json::JsonObject obj = state.open_object_at(&x, "Opaque",
                                                            xo::reflect::Reflect::require<Opaque>());
                obj.members()
                    .member("self_", px)
                    .end();
                obj.close();
            }

            REQUIRE(ss.str() == object(head<Opaque>("Opaque", 1),
                                       {entry<Opaque *>("self_", "pointer", "{\"_ref_\": 1}")}));
        } /*TEST_CASE(json-members-ref-to-an-object-a-printer-wrote)*/

        TEST_CASE("validate-cycle-terminates", "[printjson][cycle]") {
            /* validate_tp walks as print_tp does: each object once */
            reflect_node();

            PrintJson print_json;

            Node a{1, nullptr};
            Node b{2, &a};
            a.next_ = &b;

            REQUIRE_NOTHROW(print_json.validate_tp(xo::reflect::Reflect::make_tp(&a)));
        } /*TEST_CASE(validate-cycle-terminates)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end PrintJsonCycle.test.cpp */
