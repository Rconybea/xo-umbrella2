/** @file PrintJsonCycle.test.cpp
 *
 *  PrintJson on graphs that nest deeply or cycle:
 *  .xo-backlog/xo-printjson/issues/02.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/init_printjson.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <catch2/catch.hpp>
#include <csignal>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::StructReflector;

    namespace ut {
        namespace {
            /** a node in a singly-linked graph: a raw pointer member, so a
             *  graph of them can be a chain, or cycle
             **/
            struct Node {
                int id_;
                Node * next_;
            };

            void reflect_node() {
                static bool s_once = []() {
                    StructReflector<Node> sr;
                    sr.reflect_member("id", &Node::id_);
                    sr.reflect_member("next", &Node::next_);
                    sr.require_complete();
                    return true;
                }();
                (void)s_once;
            }

            /** the type keys the struct printer emits for Node, after
             *  "_name_" -- as PrintJson.test.cpp's type_member<T>()
             **/
            std::string node_type_keys() {
                std::string canonical(xo::reflect::type_name<Node>());
                return ", \"_canonical_type_\": \"" + canonical + "\""
                    + ", \"_short_type_\": \""
                    + xo::reflect::TypeDescrBase::make_short_name(canonical) + "\"";
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

            REQUIRE(ss.str() == ("{\"_name_\": \"Node\"" + node_type_keys()
                                 + ", \"id\": 1, \"next\": {\"_name_\": \"Node\"" + node_type_keys()
                                 + ", \"id\": 2, \"next\": null}}"));
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

        TEST_CASE("print-json-cycle-aborts", "[printjson][cycle]") {
            /* a -> b -> a.  Until each object prints once (issues/02 step
             * 3), a cycle nests until the limit -- the default one
             */
            reflect_node();

            ChildOutcome x = run_in_child([]() {
                PrintJson print_json;

                Node a{1, nullptr};
                Node b{2, &a};
                a.next_ = &b;

                std::stringstream ss;
                print_json.print(a, &ss);
            });

            INFO(x.stderr_.substr(0, 2000));
            REQUIRE(x.signal_ == SIGABRT);
            REQUIRE(x.stderr_.find("PrintJson: nesting would exceed max_depth ("
                                   + std::to_string(PrintJson::c_default_max_depth) + ")")
                    != std::string::npos);
        } /*TEST_CASE(print-json-cycle-aborts)*/

        TEST_CASE("print-json-self-loop-aborts", "[printjson][cycle]") {
            reflect_node();

            ChildOutcome x = run_in_child([]() {
                PrintJson print_json;

                Node a{1, nullptr};
                a.next_ = &a;

                std::stringstream ss;
                print_json.print(a, &ss);
            });

            INFO(x.stderr_.substr(0, 2000));
            REQUIRE(x.signal_ == SIGABRT);
            REQUIRE(x.stderr_.find("PrintJson: nesting would exceed max_depth (") != std::string::npos);
        } /*TEST_CASE(print-json-self-loop-aborts)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end PrintJsonCycle.test.cpp */
