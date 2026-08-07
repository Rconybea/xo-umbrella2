/* GcStatistics.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "GcStatistics.hpp"
#include <xo/ppsink/pretty_array.hpp>    /* Prettifier<std::array<T,N>> for gen_v_ */
#include <xo/ppsink/PrettyVector.hpp>   /* Prettifier<std::vector<T>> */
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/tag_ostream.hpp>    /* os << xtag(..) in display() */
#include <ostream>

namespace xo {
    namespace gc {
        /* NB in namespace xo::gc, not namespace xo -- one scope in, so an
         * unqualified xtag cannot become ambiguous with legacy xo::xtag if
         * some header in this TU makes it visible.  xrtag -> xtag: ppsink's
         * xtag does not escape, matching legacy tagstyle::raw.
         */
        using xo::pp::xtag;

        void
        PerGenerationStatistics::include_gc(std::size_t alloc_z,
                                            std::size_t before_z,
                                            std::size_t after_z,
                                            std::size_t promote_z)
        {
            this->update_snapshot(after_z);

            //++n_gc_;
            new_alloc_z_ += alloc_z;
            scanned_z_   += before_z;
            survive_z_   += after_z;
            promote_z_   += promote_z;
        }

        void
        PerGenerationStatistics::update_snapshot(std::size_t after_z)
        {
            used_z_ = after_z;
        }

        void
        PerGenerationStatistics::display(std::ostream & os) const
        {
            os << "<PerGenerationStatistics"
               << xtag("used", used_z_)
               << xtag("n_gc", n_gc_)
               << xtag("new_alloc_z", new_alloc_z_)
               << xtag("scanned_z", scanned_z_)
               << xtag("survive_z", survive_z_)
               << xtag("promote_z", promote_z_)
               << ">";
        }

        void
        GcStatistics::begin_gc(generation upto,
                               std::size_t new_alloc)
        {
            ++(this->gen_v_[static_cast<std::size_t>(upto)].n_gc_);
            this->total_allocated_ += new_alloc;
            this->total_promoted_sab_ = total_promoted_;
        }

        void
        GcStatistics::include_gc(generation upto,
                                 std::size_t alloc_z,
                                 std::size_t before_z,
                                 std::size_t after_z,
                                 std::size_t promote_z)
        {
            gen_v_[static_cast<std::size_t>(upto)].include_gc(alloc_z, before_z, after_z, promote_z);
        }

        void
        GcStatistics::update_snapshot(generation upto,
                                      std::size_t after_z)
        {
            gen_v_[static_cast<std::size_t>(upto)].update_snapshot(after_z);
        }

        void
        GcStatistics::display(std::ostream & os) const
        {
            os << "<GcStatistics"
               << xtag("gen_v", gen_v_)
               << xtag("total_allocated", total_allocated_)
               << xtag("total_promoted_sab", total_promoted_sab_)
                // total_promoted
                // n_mtuation
                // n_logged_mutation
                // n_xgen_mutation
                // n_xckp_mutation
                // << xtag("per_type_stats", per_type_stats_)
               << ">";
        }

        void
        GcStatisticsExt::display(std::ostream & os) const
        {
            os << "<GcStatisticsExt"
               << xtag("gen_v", gen_v_)
               << xtag("total_allocated", total_allocated_)
               << xtag("total_promoted_sab", total_promoted_)
               << xtag("nursery_z", nursery_z_)
               << xtag("nursery_before_ckp_z", nursery_before_checkpoint_z_)
               << xtag("nursery_after_ckp_z", nursery_after_checkpoint_z_)
               << xtag("tenured_z", tenured_z_)
               << xtag("n_mutation", n_mutation_)
               << xtag("n_logged_mutation", n_logged_mutation_)
               << xtag("n_xgen_mutation", n_xgen_mutation_)
               << xtag("n_xckp_mutation", n_xckp_mutation_)
                // << xtag("per_type_stats", per_type_stats_)
               << ">";
        }

        float
        GcStatisticsHistoryItem::collection_rate() const {
            using namespace xo::qty::qty;

            float gz = this->garbage_z();

            auto dt_nanos = this->dt_.with_repr<float>();
            auto dt_sec = dt_nanos.rescale_ext<xo::qty::u::second>();
            auto rate = gz / dt_sec;
            float retval = rate.scale();

            //scope log(XO_DEBUG_(true));
            //log && log(xtag("gz", gz), xtag("dt_sec", dt_sec), xtag("rate", rate), xtag("rate/sec", retval));

            return retval;
        }

        void
        GcStatisticsHistoryItem::display(std::ostream & os) const
        {
            os << "<GcStatisticsHistoryItem"
               << xtag("upto", upto_)
               << xtag("survive_z", survive_z_)
               << xtag("promote_z", promote_z_)
               << xtag("persist_z", persist_z_)
               << xtag("effort_z", effort_z_)
               << xtag("garbage0_z", garbage0_z_)
               << xtag("garbage1_z", garbage1_z_)
               << xtag("garbageN_z", garbageN_z_)
               << xtag("dt", dt_)
               << ">";
        }

    } /*namespace gc*/

} /*namespace xo*/

namespace xo::pp {
    void
    Prettifier<xo::gc::PerGenerationStatistics>::print(PpSink & sink, const xo::gc::PerGenerationStatistics & x)
    {
        sink.pretty_struct("PerGenerationStatistics",
                           field("used_z", x.used_z_),
                           field("n_gc", x.n_gc_),
                           field("new_alloc_z", x.new_alloc_z_),
                           field("scanned_z", x.scanned_z_),
                           field("survive_z", x.survive_z_),
                           field("promote_z", x.promote_z_));
    }

    void
    Prettifier<xo::gc::GcStatistics>::print(PpSink & sink, const xo::gc::GcStatistics & x)
    {
        sink.pretty_struct("GcStatistics",
                           field("gen_v", x.gen_v_),
                           field("total_allocated", x.total_allocated_),
                           field("total_promoted_sab", x.total_promoted_sab_),
                           field("total_promoted", x.total_promoted_),
                           field("n_mutation", x.n_mutation_),
                           field("n_logged_mutation", x.n_logged_mutation_),
                           field("n_xgen_mutation", x.n_xgen_mutation_),
                           field("n_xckp_mutation", x.n_xckp_mutation_));
    }

    void
    Prettifier<xo::gc::GcStatisticsExt>::print(PpSink & sink, const xo::gc::GcStatisticsExt & x)
    {
        sink.pretty_struct("GcStatisticsExt",
                           field("gen_v", x.gen_v_),
                           field("total_allocated", x.total_allocated_),
                           field("total_promoted_sab", x.total_promoted_sab_),
                           field("total_promoted", x.total_promoted_),
                           field("n_mutation", x.n_mutation_),
                           field("n_logged_mutation", x.n_logged_mutation_),
                           field("n_xgen_mutation", x.n_xgen_mutation_),
                           field("n_xckp_mutation", x.n_xckp_mutation_),
                           field("nursery_z", x.nursery_z_),
                           field("nursery_before_checkpoint_z", x.nursery_before_checkpoint_z_),
                           field("nursery_after_checkpoint_z", x.nursery_after_checkpoint_z_),
                           field("tenured_z", x.tenured_z_));
    }

    void
    Prettifier<xo::gc::GcStatisticsHistoryItem>::print(PpSink & sink, const xo::gc::GcStatisticsHistoryItem & x)
    {
        /* bind to a local: field() captures by reference (like legacy
         * refrtag), and gen2str() returns by value.
         */
        const char * const upto_str = gen2str(x.upto_);

        sink.pretty_struct("GcStatisticsHistoryItem",
                           field("upto", upto_str),
                           field("survive_z", x.survive_z_),
                           field("promote_z", x.promote_z_),
                           field("persist_z", x.persist_z_),
                           field("effort_z", x.effort_z_),
                           field("garbage0_z", x.garbage0_z_),
                           field("garbage1_z", x.garbage1_z_),
                           field("garbageN_z", x.garbageN_z_),
                           field("dt", x.dt_));
    }
} /*namespace xo::pp*/
