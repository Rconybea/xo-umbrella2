/** @file TempPpSink.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  TempPpSink caches one sink per thread.  These cases pin WHEN that cache is
 *  allowed to survive: across calls, yes; across a change of PpSinkFactory,
 *  no.
 *
 *  The second is the interesting one.  A thread that renders anything before
 *  the application installs its factory gets the low-level fallback, and
 *  before this was pinned it kept that fallback for the rest of its life --
 *  so an app that configured after any rendering had happened silently lost
 *  pretty-printing on that thread, with nothing to see but flat output.
 **/

#include "MarkSink.hpp"
#include <xo/ppsink/TempPpSink.hpp>
#include <xo/ppsink/PpSinkFactory.hpp>
#include <xo/ppsink/pretty.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace {
    using xo::pp::PpSink;
    using xo::pp::PpSinkFactory;
    using xo::pp::TempPpSink;

    /** factory producing a non-flat sink, standing in for PrettySinkFactory --
     *  which cannot be used here, since it lives in xo-indentlog2, one level
     *  up.  What matters to TempPpSink is only that it is a DIFFERENT factory
     *  handing out a DIFFERENT sink type.
     **/
    class MarkSinkFactory : public PpSinkFactory {
    public:
        bool is_flat() const override { return false; }

        std::unique_ptr<PpSink> create(ColorSelect, SinkOutput) override {
            return std::make_unique<ut::MarkSink>(os_);
        }

    private:
        std::ostringstream os_;
    };

    /** restore the process-wide factory and this thread's cached sink, so a
     *  case cannot leak either into the rest of the binary
     **/
    class factory_guard {
    public:
        factory_guard() : orig_{&PpSinkFactory::instance()} {}
        ~factory_guard() {
            PpSinkFactory::set_instance(orig_);
            TempPpSink::reset_local(nullptr);
        }

    private:
        PpSinkFactory * orig_;
    };
}

TEST_CASE("TempPpSink-local-is-cached", "[ppsink][TempPpSink]")
{
    factory_guard guard;

    TempPpSink::reset_local(nullptr);

    REQUIRE(TempPpSink::check_local() == nullptr);   /* nothing until asked */

    PpSink & first = TempPpSink::local();

    REQUIRE(TempPpSink::check_local() == &first);
    REQUIRE(&TempPpSink::local() == &first);         /* same sink, not a new one */
}

TEST_CASE("TempPpSink-follows-a-change-of-factory", "[ppsink][TempPpSink]")
{
    factory_guard guard;

    TempPpSink::reset_local(nullptr);

    /* rendering before the app installs its factory: the fallback answers */
    TempPpSink::local();
    REQUIRE(TempPpSink::is_flat());

    MarkSinkFactory mark_factory;
    PpSinkFactory::set_instance(&mark_factory);

    /* the cached fallback must not outlive the factory that produced it */
    TempPpSink::local();
    REQUIRE(!TempPpSink::is_flat());
}

TEST_CASE("TempPpSink-renders-through-the-fallback", "[ppsink][TempPpSink]")
{
    factory_guard guard;

    TempPpSink::reset_local(nullptr);

    /* the fallback answers before an application installs its factory, so
     * pp2str() has to WORK there -- not merely not crash.  SinkOutput::
     * k_memory is what TempPpSink asks for, and a sink that honours it has
     * somewhere to put the bytes.
     */
    REQUIRE(TempPpSink::is_flat() == false);   /* nothing built yet */

    std::string s = TempPpSink::pp2str(42);

    REQUIRE(TempPpSink::is_flat());
    REQUIRE(s == "42");
}

TEST_CASE("TempPpSink-does-not-accumulate-across-calls", "[ppsink][TempPpSink]")
{
    factory_guard guard;

    TempPpSink::reset_local(nullptr);

    /* the sink is reused, so each render must start empty: complete() has to
     * reclaim the buffer, or the second call returns the first's text too
     */
    REQUIRE(TempPpSink::pp2str(42) == "42");
    REQUIRE(TempPpSink::pp2str(43) == "43");
    REQUIRE(TempPpSink::pp2str(44) == "44");
}

TEST_CASE("TempPpSink-reset-local-survives-until-the-factory-changes",
          "[ppsink][TempPpSink]")
{
    factory_guard guard;

    MarkSinkFactory mark_factory;
    PpSinkFactory::set_instance(&mark_factory);
    TempPpSink::reset_local(nullptr);

    /* an explicitly installed sink is the caller's choice, so local() keeps it
     * rather than rebuilding from the factory on the next call
     */
    auto sbuf = std::ostringstream();
    auto * supplied = new ut::MarkSink(sbuf);

    TempPpSink::reset_local(std::unique_ptr<PpSink>(supplied));

    REQUIRE(&TempPpSink::local() == supplied);
}

/* end TempPpSink.test.cpp */
