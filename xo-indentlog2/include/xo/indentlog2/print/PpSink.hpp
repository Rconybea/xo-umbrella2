/** @file PpSink.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <string_view>
#include <cstdint>

namespace xo::print {
    class PpSink;

    /** @brief RAII convenience class to ensure ppsink.stream_commit() invoked
     **/
    class PpSinkInserter {
    public:
        explicit PpSinkInserter(PpSink * pps, std::ostream * os);
        PpSinkInserter(const PpSinkInserter &) = delete;
        ~PpSinkInserter();

        std::ostream & os() { return *os_; }

        /** finalizer.  Will be called at least once. Idempotent; dtor invokes **/
        void finish();

        PpSinkInserter & operator= (const PpSinkInserter & x) = delete;

    private:
        /** pretty-printer api **/
        PpSink * ppsink_ = nullptr;
        /** stream api.  Writing to @ref stream_ appends to @ref sink_ **/
        std::ostream * os_ = nullptr;
    };

    /** convenience so given a PpSinkInserter:
     *    PpSinkInserter ins = ppsink.stream_open(..);
     *  we can write
     *    ins << x << ...;
     *  instead of
     *    ins.os() << x << ...;
     **/
    template <typename T>
    inline PpSinkInserter &
    operator<<(PpSinkInserter & ins, const T & x) {
        ins.os() << x;
        return ins;
    }

    /** @brief Interface for a stream with pretty-printing support
     *
     *  Defer using faceted object model, so we can use this
     *  pretty-printing during implementation of xo-facet itself.
     *  May revisit later.
     **/
    class PpSink {
    public:
        using uint32_t = std::uint32_t;

    public:
        /** write string comprising contents of @p s **/
        virtual PpSink & put(std::string_view s) = 0;
        /** begin group of nested items.
         *  Sequence begin,[string*,split]*,end
         **/
        virtual PpSink & begin() = 0;
        /** optional split. Marks spot where pretty printer
         *  may introduce line break to fit
         **/
        virtual PpSink & split() = 0;
        /** end group of nested items previously  introduced with begin() **/
        virtual PpSink & end() = 0;
        /** temporary stream to fill a string token.
         *  Token will be completed by next call to stream_commit().
         *  Example:
         *    auto ins = sink.stream_open(256);
         *    ins.os() << foo << ..;
         *    ins.finish();
         *  is equivalent (in effect) to
         *    stringstream ss;
         *    ss << foo << ..;
         *    sink.put(ss.str());
         *  but avoids heap allocation + copy
         **/
        virtual PpSinkInserter stream_open(uint32_t min_z) = 0;
        /** complete string started by stream_open() **/
        virtual void stream_commit() = 0;

    };


} /*namespace xo::print*/

/* end PpSink.hpp */
