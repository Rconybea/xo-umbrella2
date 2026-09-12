/** @file PpSinkFactory.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "PpSink.hpp"

namespace xo::pp {

    class PpSink;

    /** @brief specify default sink for logging.
     *
     *  In practice will be either FlatSink (simple, low-level)
     *  or PrettySink (pretty-printing).
     *
     *  Design Note: using singleton pattern here because this feature
     *  arrives at very low-level, before we have app context
     **/
    class PpSinkFactory {
    public:
        /** control PpStyle color **/
        enum class ColorSelect {
            /** do not use color escapes (see @ref PpStyle::plain) **/
            k_plain,
            /** use default color escapes (see @ref PpStyle::colored) **/
            k_colored,
        };

        /** control PpSink destination **/
        enum class SinkOutput {
            /** Writes to in-memory buffer **/
            k_memory,
            /** Writes to console **/
            k_clog,
        };

    public:
        static PpSinkFactory & instance() { return *s_instance; }
        /** establish factory instance **/
        static PpSinkFactory & set_instance(PpSinkFactory * x);

        /** FlatSinkFactory -> true; PrettySinkFactory -> false **/
        virtual bool is_flat() const = 0;

        /** Create new default sink.
         *  May use color escapes; expected to write to stdout.
         **/
        virtual std::unique_ptr<PpSink> create(ColorSelect c, SinkOutput d) = 0;

    private:
        /** (singleton) default sink factory **/
        static PpSinkFactory * s_instance;
    };
} /*namespace xo::pp*/

/* end PpSinkFactory.hpp */
