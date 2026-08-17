/** @file Refcounted_pp.hpp
 *
 *  ppsink Prettifier<> for rp<T> / bp<T>: forward to the pointee, so a
 *  refcounted handle pretty-prints as whatever it points at.
 *
 *  The ppsink counterpart to legacy pretty_refcnt.hpp (which supplies the
 *  equivalent ppdetail<> forwarders).  Like those, these are FORWARDERS -- they
 *  carry no layout of their own, they just unwrap the pointer -- so a type T
 *  that has a Prettifier<T> automatically pretty-prints through rp<T>.
 *
 *  WITHOUT this header, rp<T> falls through ppsink's leaf path to
 *  operator<<(std::ostream&, intrusive_ptr<T> const&) (Refcounted.hpp), which
 *  flattens the pointee through an ostream and discards all group structure.
 *  That is silent: the output is still readable, it just never wraps.  So
 *  include this wherever a struct field holds an rp<T>/bp<T> whose pointee is
 *  itself struct-shaped.
 *
 *  Null renders as "<nullptr T>", matching both the legacy ppdetail forwarder
 *  and the ostream inserter.
 **/

#pragma once

#include "Refcounted.hpp"
#include <xo/ppsink/pretty.hpp>

namespace xo::pp {
    template <typename T>
    struct Prettifier<xo::ref::intrusive_ptr<T>> {
        static void print(PpSink & sink, const xo::ref::intrusive_ptr<T> & x) {
            if (const T * p = x.get()) {
                sink.pp(*p);
            } else {
                sink.put("<nullptr ");
                sink.put(xo::reflect::type_name<T>());
                sink.put(">");
            }
        }
    };

    template <typename T>
    struct Prettifier<xo::ref::Borrow<T>> {
        static void print(PpSink & sink, xo::ref::Borrow<T> x) {
            if (const T * p = x.get()) {
                sink.pp(*p);
            } else {
                sink.put("<nullptr ");
                sink.put(xo::reflect::type_name<T>());
                sink.put(">");
            }
        }
    };

} /*namespace xo::pp*/

/* end Refcounted_pp.hpp */
