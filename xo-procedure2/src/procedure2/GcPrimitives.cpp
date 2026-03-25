/** @file GcPrimitives.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "GcPrimitives.hpp"
#include <xo/object2/Integer.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/type/FunctionType.hpp>
#include <xo/type/AtomicType.hpp>
#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/generation.hpp>

namespace xo {
    using xo::mm::generation;

    namespace scm {

        // ----- request-gc -----

        obj<AGCObject>
        xfer_request_gc(obj<ARuntimeContext> rcx,
                        obj<AGCObject> upto_gco)
        {
            bool have_gc = false;

            if (rcx.collector()) {
                generation upto(obj<AGCObject,DInteger>::from(upto_gco));

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

            return DPrimitive_gco_1_gco::_make(mm, "request_gc", pm_ty, &xfer_request_gc);
        }
    }
}

/* end GcPrimitives.cpp */
