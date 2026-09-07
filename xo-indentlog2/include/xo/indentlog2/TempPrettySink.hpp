/** @file TempPrettySink.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/indentlog2/print/PrettySink.hpp"

namespace xo::pp {
    /** Per-thread scratch pretty printing.
     *
     *  Configured from Indentlog2AppCx.
     *
     *  Use:
     *    void foo() {
     *      auto target = ...;
     *
     *      std::string s = TempPrettySink::pp2str(target);
     *      ...
     *    }
     *
     *  See also:
     *  - toppstr(cfg, ...)
     *  - toppstr(...)
     *  - SinkFactory::instance()
     *
     *  1. toppstr() creates temporary sink (so pays mmap() -> unmap() cost)
     *  2. SinkFactory sets up app-level per-thread logging;
     *     may use FlatSink insteead of PrettySink.
     **/
    class TempPrettySink {
    public:
        /** Global initialization (just remembers @p cfg)
         **/
        static void init(const PpConfig & cfg);

        /** Thread-local temporary pretty sink.
         *  Allocated on demand, once per thread
         **/
        static PrettySink & local();

        template <typename T>
        static std::string pp2str(const T & x) {
            /* reference, not a copy: PrettySink has a hand-written move ctor,
             * so its copy ctor is deleted -- and a copy would be wrong anyway,
             * since the point is to reuse this thread's sink.
             */
            auto & tmp = local();

            tmp.pp(x);

            std::string retval = std::string(tmp.output());

            tmp.complete();

            return retval;
        }

    private:
        /** configuration for pretty-printing.
         *  Note that this includes arena sizing.
         **/
        static PpConfig s_ppconfig;
    };
} /*namespace include*/

/* end TempPrettySink.hpp */
