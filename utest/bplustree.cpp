/* @file bplustree.cpp */

#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include "random_tree_ops.hpp"
#include "xo/ordinaltree/BplusTree.hpp"
#include "xo/randomgen/random_seed.hpp"
#include "xo/randomgen/print.hpp"
#include "xo/indentlog/scope.hpp"
#include "catch2/catch.hpp"

namespace {
    using xo::tree::BplusTree;
    using xo::tree::BplusStdProperties;
    using xo::tree::NullReduce;
    using xo::tree::Machdep;

    using xo::rng::Seed;

    using utest::TreeUtil;

    using xo::scope;
    using xo::scope_setup;
    using xo::xtag;

    using BtreeKey = int;
    using BtreeValue = double;
    using BtreeProperties = BplusStdProperties<BtreeKey, BtreeValue, xo::tree::tags::ordinal_enabled>;
    //using BtreeProperties = BplusStdProperties<BtreeKey, BtreeValue, xo::tree::tags::ordinal_disabled>;
    using BpTree = BplusTree<BtreeKey,
                             BtreeValue,
                             NullReduce<BtreeKey>,
                             BtreeProperties>;

    /* random test data (e.g. permutation of integers [0 .. n-1]).
     * will do various tree operations using these permutations to control order
     * in which keys are presented
     */
    struct RandomTestData {
        RandomTestData(std::size_t n,
                       xo::rng::xoshiro256ss * p_rgen);

        std::vector<std::uint32_t> const & u1v() const { return u1v_; }
        std::vector<std::uint32_t> const & u2v() const { return u2v_; }
        std::vector<std::uint32_t> const & u12_v() const { return u12_v_; }

    private:
        /* a set comprising n randomly chosen elements drawn from [0 .. 2n-1].
         * here n = .u1v.size = .u2v.size
         */
        std::vector<std::uint32_t> u1v_;
        /* complement of .u1v w.r.t. [0 .. 2n-1] */
        std::vector<std::uint32_t> u2v_;
        /* .u1v + .u2v */
        std::vector<std::uint32_t> u12_v_;
    }; /*RandomTestData*/

    RandomTestData::RandomTestData(std::size_t n,
                                   xo::rng::xoshiro256ss * p_rgen)
        : u1v_(n), u2v_(n), u12_v_(2*n)
    {
        /* permutation of [0 .. 2n-1] */
        std::vector<std::uint32_t> u(2*n);

        for (std::uint32_t i=0; i<2*n; ++i)
            u[i] = i;
        std::shuffle(u.begin(), u.end(), *p_rgen);

        u1v_ = std::vector<std::uint32_t>(u.begin(), u.begin() + n);
        u2v_ = std::vector<std::uint32_t>(u.begin() + n, u.end());
        u12_v_ = std::move(u);
    } /*ctor*/

    /* representation-independent feature benchmarks for tree algorithms.
     *
     *      +------------------+
     *      |AbstractTestParams|
     *      +------------------+
     *               ^
     *               |   isa      +----------------+
     *               +------------|StdMapTestParams|   benchmark std::map<BtreeKey, BtreeValue> (bogey!)
     *               |            +----------------+
     *               |
     *               |   isa      +---------------+
     *               +------------|BtreeTestParams|    benchmark BplusTree<BtreeKey, BtreeValue, ..>
     *                            +---------------+
     */
    struct AbstractTestParams {
        virtual ~AbstractTestParams() = default;
        /* insert benchmark:
         *  1. prime tree by inserting RandomTestData.u1v (random subset comprising n draws from [0 .. 2n-1])
         *  2. measure cost of inserting RandomTestData.u2v (complement of u1v w.r.t [0 .. 2n-1])
         */
        virtual void run_insert_benchmark(RandomTestData const & random_testdata) const = 0;
        virtual void run_erase_benchmark(RandomTestData const & random_testdata) const = 0;
        virtual void run_lookup_benchmark(RandomTestData const & random_testdata) const = 0;
        virtual void run_traverse_benchmark(RandomTestData const & random_testdata) const = 0;
    };

    struct StdMapTestParams : public AbstractTestParams {
        StdMapTestParams(char const * name)
            : test_name_{name} {}

        /* 1. make map containing keys in random_testdata.u1v.
         * 2. during construction, interleave inserts against a temporary map,
         *    to spoil sequential heap allocation (i.e. simulate fragmentation)
         */
        std::map<BtreeKey, BtreeValue> make_random_map1(RandomTestData const & random_testdata) const {
            std::map<BtreeKey, BtreeValue> tree;
            /* 2nd tree to interfere with locality */
            std::map<BtreeKey, BtreeValue> tree2;

            for (std::uint32_t x : random_testdata.u1v()) {
                tree.insert({x, 10*x});
                /* 2nd tree to interfere with locality */
                for (std::uint32_t y = 0; y < 8; ++y)
                    tree2.insert({8*x+y, 10*8*x+y});
            }

            return tree;
        } /*make_random_map1*/

        /* 1. make map containing keys in both random_testdata.u1v + random_testdata.u2v
         * 2. during construction, interleave inserts against a temporary map,
         *    to spoil sequential heap allocation (i.e. simulate fragmentation)
         */
        std::map<BtreeKey, BtreeValue> make_random_map12(RandomTestData const & random_testdata) const {
            std::map<BtreeKey, BtreeValue> tree;
            /* temporary tree to interfere with locality */
            std::map<BtreeKey, BtreeValue> tree2;

            for (std::uint32_t x : random_testdata.u12_v()) {
                tree.insert({x, 10*x});
                /* 2nd tree to interfere with memory locality */
                for (std::uint32_t y = 0; y < 8; ++y)
                    tree2.insert({8*x+y, 10*8*x+y});
            }

            return tree;
        } /*make_random_map12*/

        virtual void run_insert_benchmark(RandomTestData const & random_testdata) const override;
        virtual void run_erase_benchmark(RandomTestData const & random_testdata) const override;
        virtual void run_lookup_benchmark(RandomTestData const & random_testdata) const override;
        virtual void run_traverse_benchmark(RandomTestData const & random_testdata) const override;

        char const * test_name_ = nullptr;
    };

    void
    StdMapTestParams::run_insert_benchmark(RandomTestData const & random_testdata) const
    {
        /* see also: BtreeTestParams::run_insert_benchmark() */

        BENCHMARK_ADVANCED(this->test_name_)(Catch::Benchmark::Chronometer clock)
        {
            std::size_t n = random_testdata.u1v().size();

            std::map<BtreeKey, BtreeValue> tree
                = std::move(this->make_random_map1(random_testdata));

            /* benchmark additional inserts */
            clock.measure([&](int seq) {
                std::size_t key = random_testdata.u2v()[seq % n];
                double value = 10 * key;

                tree.insert({key, value});
                return tree.size();
            });
        };
    } /*run_insert_benchmark*/

    void
    StdMapTestParams::run_erase_benchmark(RandomTestData const & random_testdata) const
    {
        BENCHMARK_ADVANCED(this->test_name_)(Catch::Benchmark::Chronometer clock)
        {
            std::size_t n = random_testdata.u1v().size();

            std::map<BtreeKey, BtreeValue> tree
                = std::move(this->make_random_map12(random_testdata));;

            clock.measure([&](int seq) {
                /* catch2 decides how many times to run this lambda,
                 * in effort to get statistically valid sample.
                 *
                 * If it calls lambda n times,  then seq will increase from [0 .. n-1]
                 */

                std::size_t key = random_testdata.u1v()[seq % n];

                //std::clog << "i=" << i << std::endl;
                tree.erase(key);

                return tree.size();
            });
        };
    } /*run_erase_benchmark*/

    void
    StdMapTestParams::run_lookup_benchmark(RandomTestData const & random_testdata) const
    {
        BENCHMARK_ADVANCED(this->test_name_)(Catch::Benchmark::Chronometer clock)
        {
            std::size_t n = random_testdata.u1v().size();

            std::map<BtreeKey, BtreeValue> tree
                = std::move(this->make_random_map1(random_testdata));

            clock.measure([&](int seq) {
                /* catch2 decides how many times to run this lambda,
                 * in effort to get statistically valid sample.
                 *
                 * If it calls lambda n times,  then seq will increase from [0 .. n-1]
                 */

                std::size_t key = random_testdata.u1v()[seq % n];

                //std::clog << "i=" << i << std::endl;
                double value = tree[key];

                return value;
            });
        };
    } /*run_lookup_benchmark*/

    void
    StdMapTestParams::run_traverse_benchmark(RandomTestData const & random_testdata) const
    {
        BENCHMARK_ADVANCED(this->test_name_)(Catch::Benchmark::Chronometer clock)
        {
            std::size_t n = random_testdata.u1v().size();

            std::map<BtreeKey, BtreeValue> tree
                = std::move(this->make_random_map1(random_testdata));

            clock.measure([&](int seq) {
                /* catch2 decides how many times to run this lambda,
                 * in effort to get statistically valid sample.
                 *
                 * If it calls lambda n times,  then seq will increase from [0 .. n-1]
                 */

                std::size_t key = random_testdata.u1v()[seq % n];

                //std::clog << "i=" << i << std::endl;
                double value = tree[key];

                return value;
            });
        };
    } /*run_traverse_benchmark*/

    struct BtreeTestParams : public AbstractTestParams {
        BtreeTestParams(char const * name, std::size_t bf, bool debug_flag)
            : test_name_{name}, branching_factor_{bf}, debug_flag_{debug_flag} {}

        BpTree make_empty_bptree() const {
            BtreeProperties properties(branching_factor_,
                                       debug_flag_);
            return BpTree(properties);
        }

        /* 1. make b+ tree containing keys in random_testdata.u1v.
         * 2. during constructions, interleave inserts against a temporary b+ tree,
         *    to spoil sequential heap allocation (i.e. simulate fragmentation)
         */
        BpTree make_random_bptree1(RandomTestData const & random_testdata) const {
            BpTree bptree = this->make_empty_bptree();
            /* 2nd tree,  just to spoil memory locality */
            BpTree bptree2 = this->make_empty_bptree();

            for (std::uint32_t x : random_testdata.u1v()) {
                bptree.insert(BpTree::value_type(x, 10 * x));
                /* 2nd tree to interfere with locality */
                for (std::uint32_t y = 0; y < 8; ++y) {
                    bptree2.insert(BpTree::value_type(8*x+y, 10 * (8*x+y)));
                }
            }

            return bptree;
        } /*make_random_bptree1*/

        BpTree make_random_bptree12(RandomTestData const & random_testdata) const {
            BpTree bptree = this->make_empty_bptree();
            /* 2nd tree,  just to spoil memory locality */
            BpTree bptree2 = this->make_empty_bptree();

            for (std::uint32_t x : random_testdata.u12_v()) {
                bptree.insert(BpTree::value_type(x, 10 * x));
                /* 2nd tree to interfere with locality */
                for (std::uint32_t y = 0; y < 8; ++y) {
                    bptree2.insert(BpTree::value_type(8*x+y, 10 * (8*x+y)));
                }
            }

            return bptree;
        } /*make_random_bptree12*/

        void run_unit_test(xo::rng::xoshiro256ss * p_rgen) const;

        virtual void run_insert_benchmark(RandomTestData const & random_testdata) const override;
        virtual void run_erase_benchmark(RandomTestData const & random_testdata) const override;
        virtual void run_lookup_benchmark(RandomTestData const & random_testdata) const override;
        virtual void run_traverse_benchmark(RandomTestData const & random_testdata) const override;

        /* test (or benchmark) name -- 1st argument to catch2 TEST_CASE() / BENCHMARK() / SECTION() macro */
        char const * test_name_ = nullptr;
        /* exercise B+ tree with this branching factor */
        std::size_t branching_factor_ = 0;
        /* for benchmarks only: if true enable verbose logging of B+ tree operations.  otherwise not used */
        bool debug_flag_ = false;
    }; /*BtreeTestParams*/

    void
    BtreeTestParams::run_unit_test(xo::rng::xoshiro256ss * p_rgen) const
    {
        std::size_t branching_factor = this->branching_factor_;

        /* perform a series of tests with increasing scale */
        for (std::uint32_t n = 0; n <= 1024;) {
            if (n == 0) {
                bool ok_flag = false;

                for (std::uint32_t attention = 0; !ok_flag && (attention < 2); ++attention) {
                    ok_flag = true;

                    bool debug_flag = (attention == 1);

                    BtreeProperties properties(branching_factor,
                                               debug_flag);
                    BpTree bptree(properties);

                    scope log(XO_DEBUG2(debug_flag, "bptree"),
                              xtag("vm_page_size", Machdep::get_page_size()),
                              xtag("branching_factor", bptree.branching_factor()),
                              xtag("leaf_node_size", sizeof(BpTree::LeafNodeType)),
                              xtag("internal_node_size", sizeof(BpTree::InternalNodeType)));

                    REQUIRE_ORCAPTURE(ok_flag, debug_flag, bptree.size() == 0);
                    REQUIRE_ORCAPTURE(ok_flag, debug_flag, bptree.verify_ok(true) == true);

                    log && log(xtag("size", n));

                    ok_flag &= TreeUtil<BpTree>::check_bidirectional_iterator(0 /*dvalue - not used*/,
                                                                              debug_flag,
                                                                              bptree);

                    ok_flag &= TreeUtil<BpTree>::test_clear(debug_flag, &bptree);

                    log.end_scope();
                }
            } else {
                /* for each tree size,  do multiple trials;
                 * choosing different pseudorandom key order for each trial
                 */
                for (std::uint32_t trial = 0; trial < 10; ++trial) {
                    /* repeated trials with different rng state */

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

                        BtreeProperties properties(branching_factor,
                                                   debug_flag);
                        BpTree bptree(properties);

                        scope log(XO_DEBUG2(debug_flag, "bptree"),
                                  xtag("vm_page_size", Machdep::get_page_size()),
                                  xtag("branching_factor", bptree.branching_factor()),
                                  xtag("leaf_node_size", sizeof(BpTree::LeafNodeType)),
                                  xtag("internal_node_size", sizeof(BpTree::InternalNodeType)));

                        REQUIRE_ORCAPTURE(ok_flag, debug_flag, bptree.size() == 0);
                        REQUIRE_ORCAPTURE(ok_flag, debug_flag, bptree.verify_ok(true) == true);

                        log && log(xtag("size", n), xtag("trial", trial));

                        /* insert [0..n-1] in random order */
                        ok_flag &= TreeUtil<BpTree>::random_inserts(n, debug_flag, p_rgen, &bptree);

                        /* verification problem -> print tree */
                        log && log(xtag("bptree", (char const *)"..."));
                        if (log) bptree.print(std::cout, log.nesting_level() + 2);

                        try {
                            REQUIRE_ORCAPTURE(ok_flag, debug_flag, bptree.verify_ok(debug_flag));
                        } catch(std::exception & ex) {
                            log && log(xtag("exception", ex.what()));
                        }

                        if (properties.ordinal_enabled()) {
                            ok_flag &= TreeUtil<BpTree>::check_ordinal_lookup(0 /*dvalue*/,
                                                                              debug_flag,
                                                                              bptree);
                        }

                        /* verify inorder traverse, using iterator api */
                        ok_flag &= TreeUtil<BpTree>::check_bidirectional_iterator(0,
                                                                                  debug_flag,
                                                                                  bptree);

                        ok_flag &= TreeUtil<BpTree>::random_lookups(debug_flag,
                                                                    bptree,
                                                                    p_rgen);

                        if (properties.ordinal_enabled()) {
                            /* paranoid check that iteration / random_lookups didn't somehow disturb tree */
                            ok_flag &= TreeUtil<BpTree>::check_ordinal_lookup(0 /*dvalue*/,
                                                                              debug_flag,
                                                                              bptree);
                        }

                        /* TODO:
                         * - check_reduced_sum()
                         * - check_ordinal_lookup()
                         * - check_bidirectional_iterator()
                         * - random_updates()
                         * - check_ordinal_lookup()
                         * - check_bidirectional_iterator()
                         * - check_reduced_sum()
                         */

                        /* remove [0..n-1] in random order */
                        ok_flag &= TreeUtil<BpTree>::random_removes(debug_flag, p_rgen, &bptree);

                        /* insert [0..n-1] again,  so we can test .clear() */
                        ok_flag &= TreeUtil<BpTree>::random_inserts(n, debug_flag, p_rgen, &bptree);

                        ok_flag &= TreeUtil<BpTree>::test_clear(debug_flag, &bptree);

                        log.end_scope();
                    } /*loop over attention value*/
                } /*loop over trial#*/
            }

            if (n == 0)
                n = 1;
            else
                n = 2*n;
        }
    } /*run_unit_test*/

    void
    BtreeTestParams::run_insert_benchmark(RandomTestData const & random_testdata) const
    {
        BENCHMARK_ADVANCED(this->test_name_)(Catch::Benchmark::Chronometer clock)
        {
            std::size_t n = random_testdata.u1v().size();

            BpTree bptree = std::move(this->make_random_bptree1(random_testdata));

            /* benchmark additional inserts (don't want to benchmark on empty tree) */
            clock.measure([&](int seq) {
                /* catch2 decides how many times to run this lambda,
                 * in effort to get statistically valid sample.
                 *
                 * If it calls lambda n times,  then seq will increase from [0 .. n-1]
                 */

                std::size_t key = random_testdata.u2v()[seq % n];
                double value = 10 * key;

                bptree.insert(BpTree::value_type(key, value));

                return bptree.size();
            });
        };
    } /*run_insert_benchmark*/

    void
    BtreeTestParams::run_erase_benchmark(RandomTestData const & random_testdata) const
    {
        BENCHMARK_ADVANCED(this->test_name_)(Catch::Benchmark::Chronometer clock)
        {
            std::size_t n = random_testdata.u1v().size();

            /* b+ tree with 2n elements */
            BpTree bptree = std::move(this->make_random_bptree12(random_testdata));

            /* measure time to remove n elements */
            clock.measure([&](int seq) {
                /* catch2 decides how many times to run this lambda,
                 * in effort to get statistically valid sample.
                 *
                 * If it calls lambda n times,  then seq will increase from [0 .. n-1]
                 */

                //std::clog << "i=" << i << std::endl;
                bptree.erase(random_testdata.u1v()[seq % n]);

                return bptree.size();
            });
        };
    } /*run_erase_benchmark*/

    void
    BtreeTestParams::run_lookup_benchmark(RandomTestData const & random_testdata) const
    {
        BENCHMARK_ADVANCED(this->test_name_)(Catch::Benchmark::Chronometer clock)
        {
            std::size_t n = random_testdata.u1v().size();

            BpTree bptree = std::move(this->make_random_bptree1(random_testdata));

            /* benchmark random lookups */
            clock.measure([&](int seq) {
                /* catch2 decides how many times to run this lambda,
                 * in effort to get statistically valid sample.
                 *
                 * If it calls lambda n times,  then seq will increase from [0 .. n-1]
                 */

                std::size_t key = random_testdata.u1v()[seq % n];

                double value = bptree[key];

                return value;
            });
        };
    } /*run_lookup_benchmark*/

    void
    BtreeTestParams::run_traverse_benchmark(RandomTestData const & random_testdata) const
    {
        BENCHMARK_ADVANCED(this->test_name_)(Catch::Benchmark::Chronometer clock)
        {
            std::size_t n = random_testdata.u1v().size();

            BpTree bptree = std::move(this->make_random_bptree1(random_testdata));

            /* benchmark traverse */
            BpTree::const_iterator ix = bptree.begin();

            clock.measure([&](int seq) {
                /* catch2 decides how many times to run this lambda,
                 * in effort to get statistically valid sample.
                 *
                 * If it calls lambda n times,  then seq will increase from [0 .. n-1]
                 */

                if (seq % n == 0)
                    ix = bptree.begin();

                return ix++;
            });
        };
    } /*run_traverse_benchmark*/

    TEST_CASE("bptree", "[bplustree]") {
        uint64_t seed = 14950349842636922572UL;
        /* can reseed from /dev/random with: */
        //Seed<xo::rng::xoshiro256ss> seed;

        auto rgen = xo::rng::xoshiro256ss(seed);

        /* exercise multiple branching factors */
        std::array<BtreeTestParams, 4> const params_v
            = {{
                BtreeTestParams("bf=4",
                                4 /*branching_factor*/,
                                false /*debug_flag - not used*/),
                BtreeTestParams("bf=12",
                                12 /*branching_factor*/,
                                false /*debug_flag - not used*/),
                BtreeTestParams("bf=28",
                                28 /*branching_factor*/,
                                false /*debug_flag - not used*/),
                BtreeTestParams("bf=60",
                                60 /*branching_factor*/,
                                false /*debug_flag - not used*/)
            }};

        for (std::uint32_t i_pm = 0; i_pm < params_v.size(); ++i_pm) {
            SECTION(params_v[i_pm].test_name_) {
                params_v[i_pm].run_unit_test(&rgen);
            }
        }
    } /*TEST_CASE(bptree)*/

    /* to run:
     *   $ ./utest.tree [!benchmark]
     *
     * looks like ospage4 (1k nodes) gets best performance
     */
    TEST_CASE("bptree-benchmark", "[!benchmark]") {
        using BtreeProperties = BplusStdProperties<BtreeKey, BtreeValue>;

        /* 2 cache lines per node (though note that we're not aligning nodes on cacheline boundaries) */
        std::size_t const c_cacheline_branching_factor = 4; // BtreeProperties::default_cacheline_branching_factor();
        std::size_t const c_ospage16_branching_factor = BtreeProperties::branching_factor_for_size(Machdep::get_page_size() / 16);
        std::size_t const c_ospage8_branching_factor = BtreeProperties::branching_factor_for_size(Machdep::get_page_size() / 8);
        std::size_t const c_ospage4_branching_factor = BtreeProperties::branching_factor_for_size(Machdep::get_page_size() / 4);
        std::size_t const c_ospage2_branching_factor = BtreeProperties::branching_factor_for_size(Machdep::get_page_size() / 2);
        std::size_t const c_ospage1_branching_factor = BtreeProperties::branching_factor_for_size(Machdep::get_page_size());

        /* random seed -- we don't need deterministic behavior for benchmarking,  unless we encounter internal logic error */
        //std::uint64_t seed = 17372468046414980217UL;
        Seed<xo::rng::xoshiro256ss> seed;

        auto rgen = xo::rng::xoshiro256ss(seed);

        constexpr bool c_debug_flag = false;

        /* n keys [0 .. n-1] */
        std::uint32_t n = 25000;

        RandomTestData random_testdata(n, &rgen);

#ifdef OBSOLETE
        /* random permutation of [0..n-1] */
        std::vector<std::uint32_t> u(n);
        {
            for (std::uint32_t i=0; i<n; ++i)
                u[i] = i;
            std::shuffle(u.begin(), u.end(), rgen);
        }

        /* random permutation of [n..2n-1] */
        std::vector<std::uint32_t> u2(n);
        {
            for (std::uint32_t i=0; i<n; ++i)
                u2[i] = n+i;
            std::shuffle(u2.begin(), u2.end(), rgen);
        }
#endif

        std::clog << "rng-seed=" << seed << "\n"
                  << "opage16-branching-factor=" << c_ospage16_branching_factor << "\n"
                  << "opage16-leaf-size=" << sizeof(BpTree::LeafNodeType) + c_ospage16_branching_factor * sizeof(BpTree::LeafNodeItemType) << "\n"
                  << "opage16-internal-size=" << sizeof(BpTree::InternalNodeType) + c_ospage16_branching_factor * sizeof(BpTree::InternalNodeItemType) << "\n"
                  << "opage8-branching-factor=" << c_ospage8_branching_factor << "\n"
                  << "opage8-leaf-size=" << sizeof(BpTree::LeafNodeType) + c_ospage8_branching_factor * sizeof(BpTree::LeafNodeItemType) << "\n"
                  << "opage8-internal-size=" << sizeof(BpTree::InternalNodeType) + c_ospage8_branching_factor * sizeof(BpTree::InternalNodeItemType) << "\n"
                  << "opage4-branching-factor=" << c_ospage4_branching_factor << "\n"
                  << "opage4-leaf-size=" << sizeof(BpTree::LeafNodeType) + c_ospage4_branching_factor * sizeof(BpTree::LeafNodeItemType) << "\n"
                  << "opage4-internal-size=" << sizeof(BpTree::InternalNodeType) + c_ospage4_branching_factor * sizeof(BpTree::InternalNodeItemType) << "\n"
                  << "opage2-branching-factor=" << c_ospage2_branching_factor << "\n"
                  << "opage2-leaf-size=" << sizeof(BpTree::LeafNodeType) + c_ospage2_branching_factor * sizeof(BpTree::LeafNodeItemType) << "\n"
                  << "opage2-internal-size=" << sizeof(BpTree::InternalNodeType) + c_ospage2_branching_factor * sizeof(BpTree::InternalNodeItemType) << "\n"
                  << "opage1-branching-factor=" << c_ospage1_branching_factor << "\n"
                  << "default-branching-factor[cacheline]=" << BplusStdProperties<BtreeKey, BtreeValue>::default_cacheline_branching_factor() << "\n"
                  << "page-size=" << Machdep::get_page_size() << "\n"
                  << "cache-line-size=" << Machdep::get_cache_line_size() << "\n"
                  << "generic-node-overhead=" << sizeof(BpTree::GenericNodeType) << "\n"
                  << "leaf-node-overhead=" << sizeof(BpTree::LeafNodeType) << "\n"
                  << "leaf-node-item-size=" << sizeof(BpTree::LeafNodeItemType) << "\n"
                  << "internal-node-overhead=" << sizeof(BpTree::InternalNodeType) << "\n"
                  << "internal-node-item-size=" << sizeof(BpTree::InternalNodeItemType) << "\n"
                  << "actual-cacheline-leaf-node-size=" << sizeof(BpTree::LeafNodeType) + c_cacheline_branching_factor * sizeof(BpTree::LeafNodeItemType) << "\n"
                  << "actual-cacheline-internal-node-size=" << sizeof(BpTree::InternalNodeType) + c_cacheline_branching_factor * sizeof(BpTree::InternalNodeItemType) << "\n"
                  << "actual-ospage-leaf-node-size=" << sizeof(BpTree::LeafNodeType) + c_ospage1_branching_factor * sizeof(BpTree::LeafNodeItemType) << "\n"
                  << "actual-ospage-internal-node-size=" << sizeof(BpTree::InternalNodeType) + c_ospage1_branching_factor * sizeof(BpTree::InternalNodeItemType) << "\n"
                  << "n=" << n << "\n"
                  << std::endl;

        {
            std::array<std::unique_ptr<AbstractTestParams>, 7> const params_v
                = {{
                    std::unique_ptr<AbstractTestParams>(new StdMapTestParams("std-map-insert")),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-min-insert",
                                                                            c_cacheline_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage16-insert",
                                                                            c_ospage16_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage8-insert",
                                                                            c_ospage8_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage4-insert",
                                                                            c_ospage4_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage2-insert",
                                                                            c_ospage2_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage-insert",
                                                                            c_ospage1_branching_factor,
                                                                            false))
                }};

            /* note: w/cacheline:
             *         getting 593ms for 10^6 inserts;
             *         i.e. ~593ns each
             *       w/ospage:
             *         getting 188ms for 10^6 inserts;
             *         i.e. ~188ns each
             *         (with ospage size 4k -> branching factor 252)
             */
            for(std::uint32_t i_bm = 0; i_bm < params_v.size(); ++i_bm) {
                params_v[i_bm]->run_insert_benchmark(random_testdata);
            }
        }

        {
            std::array<std::unique_ptr<AbstractTestParams>, 7> const params_v
                = {{
                    std::unique_ptr<AbstractTestParams>(new StdMapTestParams("std-map-erase")),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-min-remove",
                                                                            c_cacheline_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage16-remove",
                                                                            c_ospage16_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage8-remove",
                                                                            c_ospage8_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage4-remove",
                                                                            c_ospage8_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage2-remove",
                                                                            c_ospage8_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage1-remove",
                                                                            c_ospage1_branching_factor,
                                                                            false))
                }};

            /* note: cacheline: getting 72us for 10^2 removes;
             *       i.e. ~7.2ns each
             *
             *       ospage: getting 243us for 10^4 removes;
             *       i.e. ~24ns each
             */
            for (std::uint32_t i_bm = 0; i_bm < params_v.size(); ++i_bm) {
                params_v[i_bm]->run_erase_benchmark(random_testdata);
            }
        }

        {
            std::array<std::unique_ptr<AbstractTestParams>, 7> const params_v
                = {{
                    std::unique_ptr<AbstractTestParams>(new StdMapTestParams("std-map-lookup")),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-min-lookup",
                                                                            c_cacheline_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage16-lookup",
                                                                            c_ospage16_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage8-lookup",
                                                                            c_ospage8_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage4-lookup",
                                                                            c_ospage4_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage2-lookup",
                                                                            c_ospage2_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage1-lookup",
                                                                            c_ospage1_branching_factor,
                                                                            false))
                }};

            /* note: cacheline:
             *         getting 850us for 10^4 lookups;
             *          -> ~85ns each
             *       ospage:
             *         getting 585us for 10^4 lookups;
             *         -> ~58ns each
             */
            for (std::uint32_t i_bm = 0; i_bm < params_v.size(); ++i_bm) {
                params_v[i_bm]->run_lookup_benchmark(random_testdata);
            }
        }

        {
            std::array<std::unique_ptr<AbstractTestParams>, 7> const params_v
                = {{
                    std::unique_ptr<AbstractTestParams>(new StdMapTestParams("std-map-traverse")),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-min-traverse",
                                                                            c_cacheline_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage16-traverse",
                                                                            c_ospage16_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage8-traverse",
                                                                            c_ospage8_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage4-traverse",
                                                                            c_ospage4_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage2-traverse",
                                                                            c_ospage2_branching_factor,
                                                                            false)),
                    std::unique_ptr<AbstractTestParams>(new BtreeTestParams("bplus-ospage1-traverse",
                                                                            c_ospage1_branching_factor,
                                                                            false))
                }};

            /* note: cacheline: getting 25us to traverse tree of size 10^4
             *       -> ~2.5ns each
             * note: ospage: getting 6us to traverse tree of size 10^4
             *       -> ~0.6ns each
             */
            for (std::uint32_t i_bm = 0; i_bm < params_v.size(); ++i_bm) {
                params_v[i_bm]->run_traverse_benchmark(random_testdata);
            }
        }

    } /*TEST_CASE(bptree-benchmark)*/
} /*namespace*/

/* end bplustree.cpp */
