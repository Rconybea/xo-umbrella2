/** @file typeseq.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "type_name.hpp"
#include <xo/ppsink/pretty.hpp>
#include <cstdint>
#include <ostream>

namespace xo {
    namespace reflect {
        struct typerecd {
            /** sentinel value **/
            typerecd() = default;

            /** type-record with specific, unique id **/
            explicit typerecd(int32_t s,
                              std::string_view n) : seqno_{s}, name_{n} {}

            /** Can't have this be constexpr.
             *  We need ids in shared libraries to be generated
             *  at load time to avoid false positives
             *
             *  Return unique id number for each type.
             *  Numbers are sequentially allocated, so can use
             *  as vector indices
             *
             *  Conversely note that built-in typeinfo may
             *  return false negatives across library boundaries
             *  when using clang.
             **/
            template <typename T>
            static typerecd recd() {
                // reminder: {armed, id} are distint for each T
                static bool s_armed = true;
                static int32_t id = 0;

                if (s_armed) {
                    s_armed = false;
                    id = require_next_id();
                }

                return typerecd(id, xo::reflect::type_name<T>());

            }

            static int32_t require_next_id() {
                static int32_t s_next_id = 0;
                return s_next_id++;
            }

            int32_t seqno() const { return seqno_; }
            std::string_view name() const { return name_; }

            /** sentinel typerecd instance **/
            static typerecd sentinel() {
                return typerecd(-1, "_%sentinel%_");
            }

        private:
            int32_t seqno_ = -1;
            std::string_view name_;
        };

        //template <typename Tag>
        //int32_t typerecd_impl<Tag>::s_next_id = 0;

        /**
         * Tag here so we can preserve header-only implementation
         * and still have static variable
         */
        struct typeseq {
            /** create sentinel value **/
            typeseq() = default;

            /** typeseq with specific unique id **/
            explicit typeseq(int32_t s) : seqno_{s} {}

            /** 'anonymous' sentinel type.
             *  Niche uses for this, e.g. untyped allocator
             **/
            static typeseq sentinel() {
                return typeseq(typerecd::sentinel().seqno());
            }

            template <typename T>
            static typeseq id() {
                return typeseq(xo::reflect::typerecd::recd<T>().seqno());
            }

            bool is_sentinel() const { return seqno_ == -1; }
            int32_t seqno() const { return seqno_; }

        private:
            int32_t seqno_ = -1;
        };

        //template <typename Tag>
        //int32_t typeseq_impl<Tag>::s_next_id = 0;

        inline bool
        operator==(const typeseq & lhs, const typeseq & rhs) {
            return lhs.seqno() == rhs.seqno();
        }

        inline bool
        operator!=(const typeseq & lhs, const typeseq & rhs) {
            return lhs.seqno() != rhs.seqno();
        }

        /** RETAINED, for now.
         *
         *  Prettifier<typeseq> (typeseq_pp.hpp) is the intended path and this
         *  inserter should eventually move to a typeseq_ostream.hpp per
         *  .xo-backlog/milestones/ostream-containment.md.  It cannot go yet:
         *  every Setup*.cpp in the facet cluster logs typeseqs through LEGACY
         *  xo-indentlog xtag, which renders via operator<< and has no
         *  Prettifier to fall back on.  Removing this builds only after those
         *  sites are swept (or xo-indentlog is deleted).
         *
         *  <ostream>, not <iostream>: the header needs the inserter's
         *  declaration, not the standard stream objects, and xo-reflectutil is
         *  used by ~50 subsystems -- so <iostream> here was instantiating
         *  std::ios_base::Init tree-wide for nothing.
         **/
        inline std::ostream &
        operator<<(std::ostream & s, const typeseq & x) {
            s << x.seqno();
            return s;
        }

    } /*namespace reflect*/
} /*namespace xo*/

namespace xo::pp {
    /** @brief structured pretty-printing for xo::reflect::typeseq. **/
    template <>
    struct Prettifier<xo::reflect::typeseq> {
        static void print(PpSink & sink, const xo::reflect::typeseq & x) {
            sink.pp(x.seqno());
        }
    };
} /*namespace xo::pp*/

/* end typeseq.hpp */
