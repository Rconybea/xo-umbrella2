/* file ObjectStatistics.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#pragma once

#include <xo/ppsink/PpSink.hpp>
#include <xo/ppsink/Prettifier.hpp>
#include <cstdint>
#include <iosfwd>
#include <vector>

namespace xo {
    namespace reflect { class TypeDescrBase; }

    namespace gc {
        enum class capture_phase {
            /** snapshot-at-beginning **/
            sab,
            /** snapshot-at-end **/
            sae,
        };

        /** @class PerObjectTypeStatistics
         *  @brief statistics for a particular object type
         *
         *  Gathered for each leaf type descended from xo::obj::Object.
         *  See @ref xo::obj::Object::self_tp
         *
         *  See @ref GC::capture_object_statistics
         *  (gathers @ref scanned_n_, @ref scanned_z_)
         **/
        struct PerObjectTypeStatistics {
            using TypeDescr = xo::reflect::TypeDescrBase const *;

            void display(std::ostream & os) const;

            /** stats here are for objects of this type **/
            TypeDescr td_ = nullptr;
            /** number of objects scanned **/
            std::size_t scanned_n_ = 0;
            /** number of bytes scanned **/
            std::size_t scanned_z_ = 0;
            /** number of objects surviving **/
            std::size_t survive_n_ = 0;
            /** number of bytes from surviving objects **/
            std::size_t survive_z_ = 0;
        };

        inline std::ostream & operator<< (std::ostream & os, const PerObjectTypeStatistics & x) {
            x.display(os);
            return os;
        }

        /** @class ObjectStatistics
         *  @brief placeholder for type-driven allocation statistics
         *
         *  Passed to @ref Object::deep_move for example
         **/
        class ObjectStatistics {
        public:
            void display(std::ostream & os) const;

            /** per-object-type statistics, indexed by TypeId **/
            std::vector<PerObjectTypeStatistics> per_type_stats_v_;
        };

        inline std::ostream & operator<< (std::ostream & os, const ObjectStatistics & x) {
            x.display(os);
            return os;
        }

    } /*namespace gc*/

} /*namespace xo*/

namespace xo::pp {
    /* structured pretty-printing.  Defined in ObjectStatistics.cpp -- these
     * need TypeDescrBase (for short_name()), which this header only
     * forward-declares.
     */
    template <>
    struct Prettifier<xo::gc::PerObjectTypeStatistics> {
        static void print(PpSink & sink, const xo::gc::PerObjectTypeStatistics & x);
    };

    template <>
    struct Prettifier<xo::gc::ObjectStatistics> {
        static void print(PpSink & sink, const xo::gc::ObjectStatistics & x);
    };
} /*namespace xo::pp*/

/* end ObjectStatistics.hpp */
