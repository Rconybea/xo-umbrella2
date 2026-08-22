/** @file Object2.test.cpp
 *
 *  Tests that target xo-object2/ features, but also rely on xo-gc/
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include <xo/gc/X1Collector.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/ListOps.hpp>
#include <xo/object2/SetupObject2.hpp>
#include <xo/stringtable2/String.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/pretty_ostream.hpp> /* xo::pp::pp_to_stream */
#include <xo/ppsink/tag_ostream.hpp>
#include <catch2/catch.hpp>
#include <iostream>

namespace ut {
    /* the ppsink logging vocabulary, for use below.  Converted from legacy
     * xo::scope / xo::xtag 2026-08-13; see
     * .xo-backlog/xo-gc/issues/01-gc-free-of-indentlog.md
     */
    using xo::pp::scope;
    using xo::pp::xtag;

    using xo::scm::SetupObject2;
    using xo::scm::ListOps;
    using xo::scm::DList;
    using xo::scm::DInteger;
    using xo::scm::DString;
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::AGCObject;
    using xo::mm::X1CollectorConfig;
    using xo::mm::DX1Collector;
    using xo::mm::ArenaConfig;
    using xo::print::APrintable;
    using xo::facet::FacetRegistry;
    using xo::facet::with_facet;
    using xo::facet::obj;
    using xo::facet::typeseq;
    using xo::pp::pp_to_stream;
    using xo::pp::toppstr;
    using xo::pp::PpConfig;
    using std::string;

    namespace {
        struct testcase_pp {
            explicit testcase_pp(size_t gc_z, size_t gc_threshold, int z,
                                 const std::string & expect_pretty)
            : gc_gen_size_{gc_z}, gc_trigger_threshold_{gc_threshold},
              expect_pretty_{expect_pretty} {
                for (int i = 0; i < z; ++i) {
                    list_.push_back(1000 + 197 * i);
                }
            }

            size_t gc_gen_size_ = 0;
            size_t gc_trigger_threshold_ = 0;
            std::vector<int> list_;
            /** OBSERVED rendering via pretty(PpSink&).
             *
             *  The last case (z=20) is where the deprecated protocol used to
             *  differ: it could only emit "(...)" for a list that did not fit
             *  on one line -- the two-pass protocol's give-up path, not a
             *  rendering decision.  DList::pretty offers break points instead
             *  and the sink lays the elements out.  Reviewed and accepted
             *  2026-08-09; the deprecated expectation was deleted with the
             *  protocol at phase E.  See
             *  .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
             **/
            std::string expect_pretty_;
        };

        std::vector<testcase_pp>
        s_testcase_v = {
            testcase_pp(16384, 8192, 0, "()"),
            testcase_pp(16384, 8192, 1, "(01000)"),
            testcase_pp(16384, 8192, 2, "(01000 1197)"),
            testcase_pp(16384, 8192, 5, "(01000 1197 01394 1591 01788)"),
            testcase_pp(16384, 8192, 10, "(01000 1197 01394 1591 01788 1985 02182 2379 02576 2773)"),
            /* the case that used to diverge -- see expect_pretty_ above */
            testcase_pp(16384, 8192, 20, R"xo((01000
  1197
  01394
  1591
  01788
  1985
  02182
  2379
  02576
  2773
  02970
  3167
  03364
  3561
  03758
  3955
  04152
  4349
  04546
  4743))xo"),

        };
    }

    TEST_CASE("printable1", "[pp][x1][list]")
    {
        constexpr bool c_debug_flag = false;
        scope log(XO_DEBUG_(c_debug_flag), "Object2 printable1 test");

        bool ok = SetupObject2::register_facets();
        REQUIRE(ok);

        xo::pp::pp_to_stream(std::cerr, FacetRegistry::instance());

        for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            log && log("printable1 test:", xtag("i_tc", i_tc));

            try {
                const testcase_pp & tc = s_testcase_v[i_tc];

                X1CollectorConfig cfg{ .name_ = "pp_test",
                                       .arena_config_ = ArenaConfig{
                                           .size_ = tc.gc_gen_size_,
                                           .store_header_flag_ = true},
                                       .object_types_z_ = 16384,
                                       .gc_trigger_v_{{tc.gc_trigger_threshold_,
                                                       tc.gc_trigger_threshold_}},
                                       .debug_flag_ = c_debug_flag
                };

                DX1Collector gc(cfg);
                auto gc_o = with_facet<AAllocator>::mkobj(&gc);
                auto  c_o = with_facet<ACollector>::mkobj(&gc);

                bool ok = SetupObject2::register_types(c_o);
                REQUIRE(ok);

                auto l0_o = ListOps::nil();

                c_o.add_gc_root(&l0_o);

                for(int ip1 = tc.list_.size(); ip1 > 0; --ip1) {
                    obj<AGCObject> elt;

                    //elt = DInteger::box<AGCObject>(gc_o, tc.list_[ip1 - 1]);

                    if (ip1 % 2 == 0) {
                        elt = DInteger::box<AGCObject>(gc_o, tc.list_[ip1 - 1]);
                    } else {
                        elt = obj<AGCObject,DString>(DString::printf(gc_o, 80, "%05d", tc.list_[ip1 - 1]));
                    }

                    l0_o  = ListOps::cons(gc_o, elt, l0_o);
                }

                obj<APrintable,DList> l0_po(static_cast<DList*>(l0_o.data()));
                REQUIRE(l0_po._typeseq() == typeseq::id<DList>());

                /* margin 80, matching the margin the expectations were
                 * observed at.  Rendered while the COLLECTOR is live and the
                 * list is a gc root -- which is what this test covers that
                 * xo-object2's phase-C tables do not: they use a plain arena.
                 */
                std::string modern = toppstr(PpConfig::scratch_plain(80), l0_po);
                INFO("i_tc=" << i_tc << " pretty=[" << modern << "]");
                CHECK(modern == tc.expect_pretty_);
            } catch (std::exception & ex) {
                std::cerr << "caught exception: " << ex.what() << std::endl;
                REQUIRE(false);
            }
        }
    } /* TEST_CASE(printable1) */
} /*namespace ut*/

/* end Object2.test.cpp */
