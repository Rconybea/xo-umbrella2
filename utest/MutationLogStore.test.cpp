/** @file MutationLogStore.test.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include <xo/gc/GCObjectStore.hpp>
#include <xo/gc/MutationLogStore.hpp>
#include <xo/gc/X1VerifyStats.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/randomgen/xoshiro256.hpp>
#include <xo/randomgen/random_seed.hpp>
#include <catch2/catch.hpp>

namespace ut {
    using xo::mm::MutationLogStore;
    using xo::mm::MutationLogConfig;
    using xo::mm::GCObjectStore;
    using xo::mm::GCObjectStoreConfig;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::mm::X1VerifyStats;
    using xo::rng::xoshiro256ss;
    using xo::rng::random_seed;
    using xo::xtag;
    using xo::scope;

    namespace {
        enum class TestGraphType {
            /* list cell pointing to itself */
            selfcycle,
            /* random object graph */
            random,
        };

        struct Testcase {
            explicit Testcase(bool debug_flag) : debug_flag_{debug_flag} {}

            /** number of generations in gco store **/
            uint32_t n_gen_ = 0;
            /** object prommotes on surviving this many gc cycles **/
            uint32_t n_survive_ = 0;
            /** size of each generation's half-space in bytes **/
            size_t gc_size_ = 0;
            /** storage for object type in bytes **/
            uint32_t object_type_z_ = 0;
            /** if true register types for gc-aware types used in unit test **/
            bool do_type_registration_ = false;
            /** storage for mutation log (mult by 3 x n_gen_) **/
            uint32_t mutation_log_z_ = 0;
            /** true if enabling mutation-log feature
             *  (load-bearing for incremental gc)
             **/
            bool mlog_enabled_flag_ = false;
            /** object graph type **/
            TestGraphType obj_graph_type_ = TestGraphType::random;
            /** #of gc-like "move all the roots" phases to perform **/
            uint32_t n_gc_loop_ = 0;
            /** first loop: #of cells in random object graph **/
            uint32_t n_i0_test_obj_ = 0;
            /** first loop: #of random assignments to attempt **/
            uint32_t n_i0_test_assign_ = 0;
            /** 2nd+later loop: #of cells in random object graph **/
            uint32_t n_i1_test_obj_ = 0;
            /** 2nd+later loop: #of random assignments to attempt **/
            uint32_t n_i1_test_assign_ = 0;
            /** true to enable debug when attempting this test case **/
            bool debug_flag_;
        };

        constexpr TestGraphType c_selfcycle = TestGraphType::selfcycle;
        constexpr TestGraphType c_random = TestGraphType::random;
        constexpr uint32_t c_report_z1 = 64 * 1024;
        constexpr uint32_t c_error_z1 = 16 * 1024;

#      define T true
#      define F false

        static std::vector<Testcase> s_testcase_v = {
        };

#      undef T
#      undef F

        class MlsFixture {
        public:
            explicit MlsFixture(const Testcase &);

            GCObjectStoreConfig gcos_config_;
            MutationLogConfig mls_config_;

            /** statistics called by GCObjectStore.verify_ok() **/
            X1VerifyStats verify_stats_;
            GCObjectStore gcos_;
            MutationLogStore mls_;
        };

        MlsFixture::MlsFixture(const Testcase & tc)
            : gcos_config_{(ArenaConfig()
                            .with_name("mlog-fixture-arena-name-notused")
                            .with_size(tc.gc_size_)
                            .with_store_header_flag(true)),
                            tc.n_gen_,
                            tc.n_survive_,
                            tc.object_type_z_,
                            tc.debug_flag_},
              mls_config_{tc.n_gen_,
                          tc.mutation_log_z_,
                          tc.mlog_enabled_flag_,
                          tc.debug_flag_},
              gcos_{gcos_config_, &verify_stats_},
              mls_{mls_config_, &gcos_}
        {}
    }

    TEST_CASE("MutationLogStore-1", "[MutationLogStore]")
    {
        constexpr bool c_debug_flag = true;
        scope log0(XO_DEBUG(c_debug_flag), "MutationLogStore test");

        std::uint64_t seed = 7988747704879432247ul;
        //random_seed(&seed);
        log0 && log0(xtag("seed", seed));

        for (size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            auto rgen = xoshiro256ss(seed + i_tc);

            const Testcase & tc = s_testcase_v[i_tc];

            scope log1(XO_DEBUG(tc.debug_flag_),
                       "testcase loop",
                       xtag("i_tc", i_tc));

            INFO(tostr(xtag("i_tc", i_tc), xtag("n_tc", n_tc)));

            MlsFixture fixture(tc);

            // TODO:
            // 1. move GCObjectStore.test.cpp
            //    shared code to separate .*pp files
            //    - gcos_testutil.*pp
            //
            // 2. add mutation log tests.  Entry points
            //    - init_mlogs()
            //    - verify_ok()
            //    - assign_member()
            //    - swap_roles()
            //    - forward_mutation_log()
        }

    }
} /*namespace ut*/

/* end MutationLogStore.test.cpp */
