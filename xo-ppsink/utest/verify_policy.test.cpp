/** @file verify_policy.test.cpp
 *
 *  exercise xo::verify_policy (moved from xo-arena into xo-ppsink) -- its
 *  report_error() dispatch over a ppsink scope + xo::pp::tostr message.
 **/

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/verify_policy.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <stdexcept>

namespace ut {
    using xo::verify_policy;
    using xo::pp::scope;
    using xo::pp::xtag;
    using xo::pp::FlatSink;
    using xo::pp::ThreadLogState;
    using std::stringstream;

    TEST_CASE("verify_policy-flags", "[verify_policy]") {
        REQUIRE(verify_policy::log_only().log_flag());
        REQUIRE_FALSE(verify_policy::log_only().throw_flag());
        REQUIRE(verify_policy::throw_only().throw_flag());
        REQUIRE_FALSE(verify_policy::throw_only().log_flag());
        REQUIRE(verify_policy::chatty().log_flag());
        REQUIRE(verify_policy::chatty().throw_flag());
    }

    TEST_CASE("verify_policy-report-error", "[verify_policy]") {
        stringstream ss;
        FlatSink sink(ss);
        ThreadLogState::log_set_sink(&sink);

        scope log(XO_ENTER0_(always));

        /* throw_only: report_error builds the message via tostr and throws it */
        verify_policy thrower = verify_policy::throw_only();
        REQUIRE_THROWS_AS(thrower.report_error(log, "boom", xtag("x", 42)),
                          std::runtime_error);

        /* silent (flags_ = 0): no throw, no log */
        verify_policy silent = verify_policy{.flags_ = 0};
        REQUIRE_FALSE(silent.report_error(log, "quiet"));

        ThreadLogState::log_set_sink(nullptr);
    }
} /*namespace ut*/

/* end verify_policy.test.cpp */
