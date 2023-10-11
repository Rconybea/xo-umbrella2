/* @file PollingReactor.test.cpp */

#include "xo/reactor/PollingReactor.hpp"
#include "xo/reactor/FifoQueue.hpp"
#include "xo/reactor/Sink.hpp"
#include "xo/randomgen/xoshiro256.hpp"
#include "xo/indentlog/print/pair.hpp"
#include "catch2/catch.hpp"

namespace xo {
    //using xo::reactor::Reactor;
    using xo::reactor::PollingReactor;
    using xo::reactor::FifoQueue;
    using xo::reactor::SinkToFunction;
    using xo::ref::rp;
    using xo::time::timeutil;
    using xo::time::seconds;
    using xo::time::utc_nanos;

/* note: trivial REQUIRE() call in else branch bc we still want
 *       catch2 to count assertions when verification succeeds
 */
#  define REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr) \
    if (catch_flag) {                                  \
        REQUIRE((expr));                               \
    } else {                                           \
        REQUIRE(true);                                 \
        ok_flag &= (expr);                             \
    }

#  define REQUIRE_ORFAIL(ok_flag, catch_flag, expr)    \
    REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr);      \
    if (!ok_flag)                                      \
        return ok_flag

    namespace {
        using TestEvent = std::pair<utc_nanos, std::uint64_t>;
        using TestQueue = FifoQueue<TestEvent>;

        struct RandomTestData {
            RandomTestData(std::size_t n,
                           xo::rng::xoshiro256ss * p_rgen);

            std::uint32_t size() const { return u1v_.size(); }
            std::vector<std::uint64_t> const & u1v() const { return u1v_; }

        private:
            /* a set of n randomly chosen elements drawn from [0 .. 2n-1] */
            std::vector<std::uint64_t> u1v_;
        };

        RandomTestData::RandomTestData(std::size_t n,
                                       xo::rng::xoshiro256ss * p_rgen)
            : u1v_(n)
        {
            std::shuffle(u1v_.begin(), u1v_.end(), *p_rgen);
        }
    } /*namespace*/

    namespace ut {
        TEST_CASE("polling0", "[reactor]") {
            rp<PollingReactor> reactor = PollingReactor::make();

            REQUIRE(reactor.get());

            for (std::uint32_t i=0; i<3; ++i) {
                INFO(xtag("i", i));
                REQUIRE(reactor->run_one() == 0);
            }
        } /*TEST_CASE(polling0)*/

        /* return true=success, false=fail */
        bool
        run_polling1_test(std::size_t n,
                          bool catch_flag,
                          xo::rng::xoshiro256ss * p_rgen)
        {
            scope log(XO_DEBUG(catch_flag));
            log && log(xtag("n", n));

            bool ok_flag = true;

            rp<PollingReactor> reactor = PollingReactor::make();
            REQUIRE_ORFAIL(ok_flag, catch_flag, reactor.get() != nullptr);

            if (ok_flag)
                reactor->set_loglevel(catch_flag
                                      ? log_level::always
                                      : log_level::error);

            rp<TestQueue> q = TestQueue::make();
            REQUIRE_ORFAIL(ok_flag, catch_flag, q.get() != nullptr);

            if (ok_flag)
                q->set_name("fifo");

            /* capture delivered events */
            std::vector<TestEvent> out_ev_v;

            auto sink_fn
                = ([&out_ev_v](TestEvent const & x) { out_ev_v.push_back(x); });

            q->add_callback(new SinkToFunction
                            <TestEvent, std::function<void (TestEvent const &)>>(sink_fn));


            reactor->add_source(q);

            /* max #of consecutive inserts */
            std::size_t max_enq = std::max(1UL, n/3);
            /* max #of consecutive removes */
            std::size_t max_deq = std::max(1UL, n/3);

            RandomTestData seq(n, p_rgen);

            q->set_debug_sim_flag(catch_flag);

            /* verify:
             * 1. queue conservation -- everything inserted gets delivered
             * 2. events consumed in the same order they where inserted
             * 3. no problem with queue being sometimes empty
             */

            utc_nanos t0 = timeutil::ymd_hms(20231011 /*ymd*/, 131300 /*hms*/);

            /* count #of events delivered by reactor */
            std::size_t n_delivered = 0;

            std::size_t i = 0;
            while ((i < seq.u1v().size()) || (n_delivered < n)) {
                /* sum of (#of enq, #of deq) attempted for this iteration */
                std::size_t n_work_attempted = 0;
                /* sum of (#of enq, #of deq) accomplished for this iteration */
                std::size_t n_work_done = 0;
                std::size_t n_enq = p_rgen->generate() % (max_enq + 1);
                std::size_t n_deq_attempted = 1 + (p_rgen->generate() % (max_deq + 1));
                std::size_t n_deq_done = 0;

                /* pick random #of elements to insert (to back of queue) */
                {
                    for (std::size_t j = 0; (j < n_enq) && (i < seq.u1v().size()); ++j) {
                        utc_nanos ti = t0 + seconds(i);

                        q->notify_ev(std::make_pair(ti, seq.u1v()[i++]));
                    }

                    n_work_attempted += n_enq;
                    n_work_done += n_enq;
                }

                /* pick random #of elements to remove (from front of queue) */
                {


                    for (std::size_t j = 0; j < n_deq_attempted; ++j)
                        n_deq_done += reactor->run_one();

                    n_work_attempted += n_deq_attempted;
                    n_work_done += n_deq_done;
                    n_delivered += n_deq_done;
                }

                log && log(xtag("i", i),
                           xtag("n", n),
                           xtag("n_work_attempted", n_work_attempted),
                           xtag("n_work_done", n_work_done),
                           xtag("n_enq", n_enq),
                           xtag("n_deq_attempted", n_deq_attempted),
                           xtag("n_deq_done", n_deq_done));

                if ((i == seq.u1v().size()) /*no more enqueues planned*/
                    && (n_work_attempted > 0)
                    && (n_work_done == 0))
                {
                    /* expect incremental progress every iteration;
                     * want unit test to always terminate
                     */
                    break;
                }
            }

            REQUIRE_ORFAIL(ok_flag, catch_flag, i == n);
            REQUIRE_ORFAIL(ok_flag, catch_flag, n_delivered == n);

            /* check events delivered 1:1 and in order */
            for (std::size_t i=0; i<n; ++i) {
                INFO(xtag("i", i));

                REQUIRE_ORFAIL(ok_flag, catch_flag, out_ev_v[i].second == seq.u1v()[i]);
            }

            return true;
        } /*run_polling1_test*/

        TEST_CASE("polling1", "[reactor]") {
            //log_config::style = function_style::streamlined;
            log_config::location_tab = 100;

            /* random data to exercise queue + reactor */

            uint64_t seed = 14950349842636922572UL;
            /* can seed instead from /dev/random with: */
            //Seed<xo::rng::xoshiro256ss> seed;
            auto rgen = xo::rng::xoshiro256ss(seed);

            for (std::size_t n = 4; n <= 1024; n *= 2) {
                bool ok_flag = false;

                for (std::uint32_t attention = 0; !ok_flag && (attention < 2); ++attention) {
                    ok_flag = true;

                    /* attention=0:
                     *   - no logging
                     *   - detect assertion failures,  but don't report them to catch
                     * attention=1:
                     *   - only runs if failure detected with attention=0
                     *   - full logging
                     *   - report to catch
                     */

                    bool debug_flag = (attention == 1);

                    ok_flag &= run_polling1_test(n, debug_flag, &rgen);
                }
            }

        } /*TEST_CASE(polling1)*/
    } /*namespace ut*/

} /*namespace xo*/


/* end PollingReactor.test.cpp */
