/** @file typeseq.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "type_name.hpp"
#include <xo/ppsink/pretty.hpp>
#include <vector>
#include <string_view>
#include <cstdint>
#include <cstddef>

namespace xo {
    namespace reflect {
        struct typerecd {
            /** sentinel value **/
            typerecd() = default;

            /** type-record with specific, unique id **/
            explicit typerecd(int32_t s,
                              std::string_view n) : seqno_{s}, name_{n} {}

            /** sentinel typerecd instance **/
            static typerecd sentinel() {
                return typerecd();
            }

            /** Get globally unique identity record for type T.
             *  Note this isn't knowable until load time for
             *  symbols in shared libraries.
             **/
            template <typename T>
            static typerecd recd() {
                /* note: The id variable may be separate for each
                 * library, depending on symbol visibility.
                 *
                 * In particular pybind libraries that
                 * instantiate this template get their own private id.
                 */
                static const typerecd recd = _by_name(type_name<T>());

                return recd;
            }

            /** Establish identity record for type @p name.
             *  O(n) may be acceptable here, since only used
             *  in implementation of @ref recd()
             **/
            static typerecd _by_name(std::string_view name);

            /** next global id# **/
            static int32_t id_count();
            /** number of entries in global typerecd table **/
            static std::size_t table_z();

            int32_t seqno() const { return seqno_; }
            std::string_view name() const { return name_; }

        private:
            /** next global type id number **/
            static std::int32_t s_next_id;
            /** globally-unique lookup table for typerecd instances. **/
            static std::vector<typerecd> s_typerecd_table_;

            int32_t seqno_ = -1;
            std::string_view name_ = "_%sentinel%_";
        };

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
                return typeseq(typerecd::recd<T>().seqno());
            }

            bool is_sentinel() const { return seqno_ == -1; }
            int32_t seqno() const { return seqno_; }

        private:
            int32_t seqno_ = -1;
        };

        inline bool
        operator==(const typeseq & lhs, const typeseq & rhs) {
            return lhs.seqno() == rhs.seqno();
        }

        inline bool
        operator!=(const typeseq & lhs, const typeseq & rhs) {
            return lhs.seqno() != rhs.seqno();
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
