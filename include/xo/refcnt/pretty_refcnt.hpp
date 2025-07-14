/* @file pretty_refcnt.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "Refcounted.hpp"
#include "xo/indentlog/print/pretty.hpp"

namespace xo {
    namespace print {
        template <>
        struct ppdetail<xo::ref::Refcount *> {
            static bool print_upto(ppstate * pps, const xo::ref::Refcount * x) {
                return ppdetail_atomic<const xo::ref::Refcount *>::print_upto(pps, x);
            }
            static void print_pretty(ppstate * pps, const xo::ref::Refcount * x) {
                ppdetail_atomic<const xo::ref::Refcount *>::print_pretty(pps, x);
            }
        };

        template <typename T>
        struct ppdetail<rp<T>> {
            static bool print_upto(ppstate * pps, const rp<T> & x) {
                if (auto p = x.get()) {
                    return ppdetail<T>::print_upto(pps, *p);
                } else {
                    /* note: degenerate case here, since never write newline for nullptr */

                    pps->write("<nullptr ");
                    pps->write(reflect::type_name<T>());
                    pps->write(">");

                    return pps->has_margin();
                }
            }

            static void print_pretty(ppstate * pps,  const rp<T> & x) {
                if (auto p = x.get()) {
                    ppdetail<T>::print_pretty(pps, *p);
                } else {
                    pps->write("<nullptr ");
                    pps->write(reflect::type_name<T>());
                    pps->write(">");
                }
            }
        };

        template <typename T>
        struct ppdetail<bp<T>> {
            static bool print_upto(ppstate * pps, const bp<T> & x) {
                if (auto p = x.get()) {
                    return ppdetail<T>::print_upto(pps, *p);
                } else {
                    /* note: degenerate case here, since never write newline for nullptr */

                    pps->write("<nullptr ");
                    pps->write(reflect::type_name<T>());
                    pps->write(">");

                    return pps->has_margin();
                }
            }

            static void print_pretty(ppstate * pps,  const bp<T> & x) {
                if (auto p = x.get()) {
                    ppdetail<T>::print_pretty(pps, *p);
                } else {
                    pps->write("<nullptr ");
                    pps->write(reflect::type_name<T>());
                    pps->write(">");
                }
            }
        };
    }
}
