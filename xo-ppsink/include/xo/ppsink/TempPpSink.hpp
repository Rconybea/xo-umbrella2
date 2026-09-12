/** @file TempPpSink.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "PpSink.hpp"

namespace xo::pp {

    class PpSinkFactory;

    /** @brief Per-thread scratch pretty printing.
     *
     *  Uses PpSinkFactory for sink implementation.
     *  Temporary sinks are indended to be usable for
     *  python __repr__ functions.
     *  It follows that they make not use control characters,
     *  except for whitespace.
     *  In particular they may not use ansi color escapes.
     *
     *  Configured from Indentlog2AppCx.
     *
     *  Use:
     *    @code
     *    void foo() {
     *      auto target = ...;
     *
     *      std::string s = TempPpSink::pp2str(target);
     *      ...
     *    }
     *    @endcode
     *
     *  See also:
     *  - toppstr(cfg, ...)
     *  - toppstr(...)
     *  - tostr0(...)
     *  - PpSinkFactory::instance()
     *
     *  1. toppstr() creates temporary sink (so pays mmap() -> unmap() cost);
     *     the same goes for tostr0().
     *  2. PpSinkFactory sets up app-level per-thread logging;
     *     in practice expect to use PrettySink;
     *     in a leveling context where xo-indentlog2 is not present (uncommon),
     *     may use FlatSink
     **/
    class TempPpSink {
    public:
        /** reset thread-local instance.
         *
         *  Adopts the current factory as the one @p pps came from, so an
         *  explicitly supplied sink survives until the factory itself
         *  changes -- see @ref local().
         **/
        static void reset_local(std::unique_ptr<PpSink> pps);

        /** Return pp sink for current thread, if established **/
        static PpSink * check_local();

        /** true iff pp sink for current thread is flat.  Implied check_local() **/
        static bool is_flat();

        /** Thread-local temporary pretty sink.
         *  Allocated on demand, once per thread.
         *
         *  Rebuilt when @ref PpSinkFactory::instance() is not the factory the
         *  cached sink came from.  Without that check a thread that rendered
         *  anything before the application installed its factory would keep
         *  the low-level fallback for the rest of its life: the sink is
         *  per-thread, so the application has no way to reach one it did not
         *  create.  Caching only within a factory generation is what makes
         *  configuring AFTER first use behave the same as configuring before.
         **/
        static PpSink & local();

        template <typename T>
        static std::string pp2str(const T & x) {
            auto & sink = local();

            sink.pp(x);

            auto tmp = sink.copy_output();

            sink.complete();

            return tmp.second;
        }

    private:
        /** thread-local instance.  Established per-thread by @ref local() **/
        static thread_local std::unique_ptr<PpSink> s_ppsink;
        /** factory @ref s_ppsink was built from; null when there is no sink.
         *  Compared by identity, so a factory that is replaced and later
         *  restored still counts as the same one.
         **/
        static thread_local PpSinkFactory * s_factory;
    };

} /*namespace xo::pp*/

/* end TempPpSink.hpp */
