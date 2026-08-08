/* @file pretty_localenv.hpp */

#pragma once

#include "LocalSymtab.hpp"
#include <xo/refcnt/Refcounted_pp.hpp>   /* Prettifier<rp<T>> forwarder */
#include <xo/ppsink/pretty.hpp>

namespace xo::pp {
    template <>
    struct Prettifier<xo::scm::SymbolTable> {
        static void print(PpSink & sink, const xo::scm::SymbolTable & x) {
            x.pretty(sink);
        }
    };

    template <>
    struct Prettifier<xo::scm::LocalSymtab> {
        static void print(PpSink & sink, const xo::scm::LocalSymtab & x) {
            x.pretty(sink);
        }
    };

    template <>
    struct Prettifier<xo::scm::LocalSymtab *> {
        static void print(PpSink & sink, const xo::scm::LocalSymtab * x) {
            if (x) {
                x->pretty(sink);
            } else {
                sink.put("<nullptr ");
                sink.put(xo::reflect::type_name<xo::scm::LocalSymtab>());
                sink.put(">");
            }
        }
    };
} /*namespace xo::pp*/

/* end pretty_localenv.hpp */
