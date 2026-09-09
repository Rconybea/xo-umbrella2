/** @file pretty.test.cpp
 *
 *  Pin the rendered form of xo-arena's Prettifier specializations.
 *
 *  Through FlatSink (xo-ppsink) rather than PrettySink: the latter lives in
 *  xo-indentlog2, above this subsystem.  So these assert the flat text -- what
 *  the fields are, in what order, and which are elided -- and say nothing
 *  about line breaking, which is the pretty printer's business and is covered
 *  there.
 **/

#include "MemorySizeInfo.hpp"
#include "AllocHeader.hpp"
#include "ArenaConfig.hpp"
#include "AllocHeaderConfig.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/pretty.hpp>   /* PpSink::pp */
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    using xo::pp::FlatSink;
    using xo::mm::MemorySizeInfo;
    using xo::mm::MemorySizeDetail;
    using xo::mm::AllocHeader;
    using xo::mm::ArenaConfig;
    using xo::mm::ArenaNameStr;
    using xo::mm::AllocHeaderConfig;
    using xo::reflect::typeseq;
    using std::string;

    namespace {
        /** render @p x through its Prettifier, and return the flat text.
         *
         *  PpStyle::plain(): the default style colors tag names, and the
         *  escapes would be pinned into every expectation here.
         **/
        template <typename T>
        string render(const T & x) {
            std::stringbuf buf;
            {
                FlatSink sink(xo::pp::PpStyle::plain(), &buf);
                sink.pp(x);
            }
            return buf.str();
        }
    } /*namespace*/

    TEST_CASE("pretty-MemorySizeDetail", "[arena][pretty]")
    {
        MemorySizeDetail detail{typeseq::id<double>(), 5, 40};

        /* tseq is a typeseq, not a number: seqno() is what prints.  Its exact
         * value depends on registration order, so only the shape is pinned
         */
        string s = render(detail);

        REQUIRE(s.starts_with("<MemorySizeDetail :tseq "));
        REQUIRE(s.find(":n_alloc 5") != string::npos);
        REQUIRE(s.find(":z_alloc 40") != string::npos);
        REQUIRE(s.ends_with(">"));
    }

    TEST_CASE("pretty-MemorySizeInfo-empty", "[arena][pretty]")
    {
        /* the sentinel: no name, no addresses, no histogram -- every optional
         * field elided, so this is the minimum the renderer can emit
         */
        REQUIRE(render(MemorySizeInfo::sentinel())
                == "<MemorySizeInfo :used 0 :committed 0 :reserved 0>");
    }

    TEST_CASE("pretty-MemorySizeInfo-populated", "[arena][pretty]")
    {
        MemorySizeInfo::DetailArrayType detail{};
        detail[0] = MemorySizeDetail{typeseq(), 5, 40};
        detail[1] = MemorySizeDetail{typeseq::id<double>(), 5, 40};

        std::byte lo{}, hi{};

        MemorySizeInfo info("store",
                            40 /*used*/, 48 /*allocated*/,
                            4096 /*committed*/, 262144 /*reserved*/,
                            &lo, &hi, &detail);

        string s = render(info);

        REQUIRE(s.starts_with("<MemorySizeInfo :name store"));
        REQUIRE(s.find(":used 40") != string::npos);
        /* allocated is shown only because it disagrees with used */
        REQUIRE(s.find(":allocated 48") != string::npos);
        REQUIRE(s.find(":committed 4096") != string::npos);
        REQUIRE(s.find(":reserved 262144") != string::npos);
        REQUIRE(s.find(":lo 0x") != string::npos);
        REQUIRE(s.find(":hi 0x") != string::npos);
        /* the histogram itself is not rendered -- only how much there is */
        REQUIRE(s.find(":n_detail 2") != string::npos);
        REQUIRE(s.find("MemorySizeDetail") == string::npos);
    }

    TEST_CASE("pretty-MemorySizeInfo-elides-allocated-when-equal", "[arena][pretty]")
    {
        /* the common case: nothing wasted, so the second number would only
         * repeat the first
         */
        MemorySizeInfo info("store", 40, 40, 4096, 262144, nullptr, nullptr, nullptr);

        string s = render(info);

        REQUIRE(s.find(":used 40") != string::npos);
        REQUIRE(s.find(":allocated") == string::npos);
        REQUIRE(s.find(":lo") == string::npos);
        REQUIRE(s.find(":n_detail") == string::npos);
    }

    TEST_CASE("pretty-MemorySizeInfo-n_detail-counts-populated", "[arena][pretty]")
    {
        MemorySizeInfo::DetailArrayType detail{};

        MemorySizeInfo info("store", 0, 0, 0, 0, nullptr, nullptr, &detail);

        /* an all-sentinel array is 32 empty rows, not 32 rows */
        REQUIRE(info.n_detail() == 0);

        detail[0].n_alloc_ = 1;
        REQUIRE(info.n_detail() == 1);
    }

    TEST_CASE("pretty-AllocHeader", "[arena][pretty]")
    {
        /* the packed word, in hex -- decimal would say nothing, and decoding
         * it needs the AllocHeaderConfig that wrote it
         */
        REQUIRE(render(AllocHeader(0xdeadbeef))
                == "<AllocHeader :repr 0xdeadbeef>");
    }

    TEST_CASE("pretty-ArenaConfig", "[arena][pretty]")
    {
        /* pre-existing Prettifier; asserted here so the four xo-arena
         * renderings are pinned in one place
         */
        string s = render(ArenaConfig{ .name_ = ArenaNameStr::from_cstr("store"), .size_ = 4096 });

        REQUIRE(s.starts_with("<ArenaConfig :name store :size 4096"));
        /* name is optional, and elided when empty */
        REQUIRE(render(ArenaConfig{ .size_ = 4096 }).find(":name") == string::npos);
    }

    TEST_CASE("pretty-AllocHeaderConfig", "[arena][pretty]")
    {
        string s = render(AllocHeaderConfig());

        REQUIRE(s.starts_with("<AllocHeaderConfig :tseq_bits "));
        /* uint8_t members must reach the sink as numbers, not characters */
        REQUIRE(s.find(":tseq_bits 24") != string::npos);
        REQUIRE(s.find(":age_bits 8") != string::npos);
        REQUIRE(s.find(":size_bits 32") != string::npos);
    }
} /*namespace ut*/

/* end pretty.test.cpp */
