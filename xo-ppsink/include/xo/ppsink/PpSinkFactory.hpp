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
        static PpSinkFactory & instance() { return *s_instance; }
        /** establish factory instance **/
        static PpSinkFactory & set_instance(PpSinkFactory * x);

        /** FlatSinkFactory -> true; PrettySinkFactory -> false **/
        virtual bool is_flat() const = 0;

        /** create sink **/
        virtual std::unique_ptr<PpSink> create() = 0;

    private:
        /** (singleton) default sink factory **/
        static PpSinkFactory * s_instance;
    };

} /*namespace xo::pp*/

/* end PpSinkFactory.hpp */
