/** @file GcPrimitives.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "GcPrimitives.hpp"
#include <xo/object2/Integer.hpp>
#include <xo/object2/Boolean.hpp>
//#include <xo/gc/X1Collector.hpp>
#include <xo/type/FunctionType.hpp>
#include <xo/type/AtomicType.hpp>
#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/Generation.hpp>

namespace xo {
    using xo::mm::ACollector;
    //using xo::mm::DX1Collector;
    using xo::mm::Generation;

    namespace scm {

        // ----- report-gc-status -----

        obj<AGCObject>
        xfer_report_gc_statistics(obj<ARuntimeContext> rcx)
        {
            if (rcx.collector()) {
                obj<AGCObject> stats;
                bool ok = rcx.collector().report_statistics(rcx.allocator(),
                                                            rcx.error_allocator(),
                                                            &stats);

                if (ok && stats)
                    return stats;
            }

            return DBoolean::box(rcx.allocator(), false);
        }

        DPrimitive_gco_0 *
        GcPrimitives::make_report_gc_statistics_pm(obj<AAllocator> mm,
                                                   StringTable * stbl)
        {
            (void)stbl;

            auto any_ty = DAtomicType::make(mm, Metatype::t_any());
            auto pm_ty = obj<AType,DFunctionType>(DFunctionType::_make(mm, any_ty));

            return DPrimitive_gco_0::_make(mm, "report-gc-statistics", pm_ty, &xfer_report_gc_statistics);
        }

        // ----- report-gc-object-types -----

        obj<AGCObject>
        xfer_report_gc_object_types(obj<ARuntimeContext> rcx)
        {
            if (rcx.collector()) {
                obj<AGCObject> stats;
                bool ok = rcx.collector().report_object_types(rcx.allocator(), rcx.error_allocator(), &stats);

                if (ok && stats)
                    return stats;
            }

            return DBoolean::box(rcx.allocator(), false);
        }

        DPrimitive_gco_0 *
        GcPrimitives::make_report_gc_object_types_pm(obj<AAllocator> mm,
                                                     StringTable * stbl)
        {
            (void)stbl;

            auto any_ty = DAtomicType::make(mm, Metatype::t_any());
            auto pm_ty = obj<AType,DFunctionType>(DFunctionType::_make(mm, any_ty));

            return DPrimitive_gco_0::_make(mm, "report-gc-object-types", pm_ty, &xfer_report_gc_object_types);

        }

        // ----- report-gc-object-ages -----

        obj<AGCObject>
        xfer_report_gc_object_ages(obj<ARuntimeContext> rcx)
        {
            if (rcx.collector()) {
                obj<AGCObject> stats;
                bool ok = rcx.collector().report_object_ages(rcx.allocator(), rcx.error_allocator(), &stats);

                if (ok && stats)
                    return stats;
            }

            return DBoolean::box(rcx.allocator(), false);
        }

        DPrimitive_gco_0 *
        GcPrimitives::make_report_gc_object_ages_pm(obj<AAllocator> mm,
                                                    StringTable * stbl)
        {
            (void)stbl;

            auto any_ty = DAtomicType::make(mm, Metatype::t_any());
            auto pm_ty = obj<AType,DFunctionType>(DFunctionType::_make(mm, any_ty));

            return DPrimitive_gco_0::_make(mm, "report-gc-object-ages", pm_ty, &xfer_report_gc_object_ages);
        }

        // ----- gc-location-of -----

        obj<AGCObject>
        xfer_gc_location_of(obj<ARuntimeContext> rcx, obj<AGCObject> gco)
        {
            std::int32_t location_code = 0;

            if (rcx.collector()) {
                location_code = rcx.collector().locate_address(gco.data());
            }

            return DInteger::box(rcx.allocator(), location_code);
        }

        DPrimitive_gco_1_gco *
        GcPrimitives::make_gc_location_of_pm(obj<AAllocator> mm,
                                             StringTable * stbl)
        {
            (void)stbl;

            auto int_ty = DAtomicType::make(mm, Metatype::t_integer());
            auto any_ty = DAtomicType::make(mm, Metatype::t_any());
            auto pm_ty = obj<AType,DFunctionType>(DFunctionType::_make(mm, int_ty, any_ty));

            return DPrimitive_gco_1_gco::_make(mm, "gc-location-of", pm_ty, &xfer_gc_location_of);
        }

        // ----- request-gc -----

        obj<AGCObject>
        xfer_request_gc(obj<ARuntimeContext> rcx,
                        obj<AGCObject> upto_gco)
        {
            bool have_gc = false;

            if (rcx.collector()) {
                Generation upto(obj<AGCObject,DInteger>::from(upto_gco));

                rcx.collector().request_gc(upto);

                have_gc = true;
            }

            return DBoolean::box(rcx.allocator(), have_gc);
        }

        DPrimitive_gco_1_gco *
        GcPrimitives::make_request_gc_pm(obj<AAllocator> mm,
                                         StringTable * stbl)
        {
            (void)stbl;

            auto int_ty = DAtomicType::make(mm, Metatype::t_integer());
            auto bool_ty = DAtomicType::make(mm, Metatype::t_bool());
            auto pm_ty = obj<AType,DFunctionType>(DFunctionType::_make(mm,
                                                                       bool_ty,
                                                                       int_ty));

            return DPrimitive_gco_1_gco::_make(mm, "request-gc", pm_ty, &xfer_request_gc);
        }
    }
}

/* end GcPrimitives.cpp */
