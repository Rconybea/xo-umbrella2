/* @file pretty_refcnt.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "Refcounted.hpp"
#include "xo/indentlog/print/pretty.hpp"

namespace xo {
    namespace print {
#ifndef ppdetail_atomic
        template <>
        struct ppdetail<xo::ref::Refcount *> {
            static bool print_pretty(const ppindentinfo & ppii, const xo::ref::Refcount * x) {
                return ppdetail_atomic<const xo::ref::Refcount *>::print_pretty(ppii, x);
            }
        };
#endif

        template <typename T>
        struct ppdetail<rp<T>> {
            static bool print_pretty(const ppindentinfo & ppii, const rp<T> & x) {
                if (ppii.upto()) {
                    if (auto p = x.get()) {
                        return ppdetail<T>::print_pretty(ppii, *p);
                    } else {
                        /* note: degenerate case here, since never write newline for nullptr */

                        ppii.pps()->write("<nullptr ");
                        ppii.pps()->write(reflect::type_name<T>());
                        ppii.pps()->write(">");

                        return ppii.pps()->has_margin();
                    }
                } else {
                    if (auto p = x.get()) {
                        ppdetail<T>::print_pretty(ppii, *p);
                    } else {
                        ppii.pps()->write("<nullptr ");
                        ppii.pps()->write(reflect::type_name<T>());
                        ppii.pps()->write(">");
                    }
                    return false;
                }
            }
        };

        template <typename T>
        struct ppdetail<bp<T>> {
            static bool print_pretty(const ppindentinfo & ppii, const bp<T> & x) {
                if (ppii.upto()) {
                    if (auto p = x.get()) {
                        return ppdetail<T>::print_pretty(ppii, *p);
                    } else {
                        /* note: degenerate case here, since never write newline for nullptr */

                        ppii.pps()->write("<nullptr ");
                        ppii.pps()->write(reflect::type_name<T>());
                        ppii.pps()->write(">");

                        return ppii.pps()->has_margin();
                    }
                } else {
                    if (auto p = x.get()) {
                        ppdetail<T>::print_pretty(ppii, *p);
                    } else {
                        ppii.pps()->write("<nullptr ");
                        ppii.pps()->write(reflect::type_name<T>());
                        ppii.pps()->write(">");
                    }
                    return false;
                }
            }
        };
    }
}
