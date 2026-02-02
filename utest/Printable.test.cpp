/** @file Printable.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "ListOps.hpp"
#include "DList.hpp"
#include "object2_register_types.hpp"
#include "object2_register_facets.hpp"

#include <xo/object2/DList.hpp>
#include <xo/object2/list/IGCObject_DList.hpp>
#include <xo/object2/list/IPrintable_DList.hpp>

#include <xo/object2/DString.hpp>
#include <xo/object2/string/IGCObject_DString.hpp>
#include <xo/object2/string/IPrintable_DString.hpp>

#include <xo/object2/DInteger.hpp>
#include <xo/object2/number/IGCObject_DInteger.hpp>

#include <xo/gc/Collector.hpp>
#include <xo/gc/DX1Collector.hpp>
#include <xo/gc/detail/IAllocator_DX1Collector.hpp>
#include <xo/gc/detail/ICollector_DX1Collector.hpp>

#include <xo/printable2/Printable.hpp>

#include <xo/facet/FacetRegistry.hpp>

#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>

#include <catch2/catch.hpp>

namespace ut {
    using xo::scm::object2_register_types;
    using xo::scm::object2_register_facets;
    using xo::scm::ListOps;
    using xo::scm::DList;
    using xo::scm::DInteger;
    using xo::scm::DString;
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::AGCObject;
    using xo::mm::DX1Collector;
    using xo::mm::X1CollectorConfig;
    using xo::mm::ArenaConfig;
    using xo::print::APrintable;
    using xo::facet::FacetRegistry;
    using xo::facet::with_facet;
    using xo::facet::obj;
    using xo::facet::typeseq;
    using xo::print::ppstate_standalone;
    using xo::print::ppconfig;
    using xo::scope;
    using xo::xtag;
    using std::string;

    namespace {
        struct testcase_pp {
            explicit testcase_pp(size_t gc_z, size_t gc_threshold, int z, const std::string & expected)
            : gc_gen_size_{gc_z}, gc_trigger_threshold_{gc_threshold}, expected_{expected} {
                for (int i = 0; i < z; ++i) {
                    list_.push_back(1000 + 197 * i);
                }
            }

            size_t gc_gen_size_ = 0;
            size_t gc_trigger_threshold_ = 0;
            std::vector<int> list_;
            std::string expected_;
        };

        std::vector<testcase_pp>
        s_testcase_v = {
            testcase_pp(16384, 8192, 0, "()"),
            testcase_pp(16384, 8192, 1, "(01000)"),
            testcase_pp(16384, 8192, 2, "(01000 1197)"),
            testcase_pp(16384, 8192, 5, "(01000 1197 01394 1591 01788)"),
            testcase_pp(16384, 8192, 10, "(01000 1197 01394 1591 01788 1985 02182 2379 02576 2773)"),
            testcase_pp(16384, 8192, 20, "(...)"),
        };
    }

    TEST_CASE("printable1", "[pp][x1][list]")
    {
        constexpr bool c_debug_flag = true;
        scope log(XO_DEBUG(c_debug_flag));

        bool ok = object2_register_facets();
        REQUIRE(ok);

        FacetRegistry::instance().dump(&std::cerr);

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

                bool ok = object2_register_types(c_o);
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

                // TODO: log_streambuf using DArena
                std::stringstream ss;
                ppconfig ppc;
                ppstate_standalone pps(&ss, 0, &ppc);

                obj<APrintable,DList> l0_po(static_cast<DList*>(l0_o.data()));
                REQUIRE(l0_po._typeseq() == typeseq::id<DList>());

                pps.pretty(l0_po);

                CHECK(ss.str() == string(tc.expected_));
            } catch (std::exception & ex) {
                std::cerr << "caught exception: " << ex.what() << std::endl;
                REQUIRE(false);
            }
        }
    } /* TEST_CASE(printable1) */
}

/* end Printable.test.cpp */
