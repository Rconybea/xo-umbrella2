/** @file SetupProcedure2.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "SetupProcedure2.hpp"
#include "Procedure.hpp"
#include "ObjectPrimitives.hpp"
#include "GcPrimitives.hpp"
#include "SimpleRcx.hpp"
#include "Primitive_gco_0.hpp"
#include "Primitive_gco_1_gco.hpp"
#include "Primitive_gco_2_gco_gco.hpp"
#include "Primitive_gco_2_dict_string.hpp"
#include "Primitive_gco_3_dict_string_gco.hpp"

#include <xo/alloc2/GCObject.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::mm::AAllocator;
    using xo::facet::FacetRegistry;
    using xo::facet::impl_for;
    using xo::facet::typeseq;
    using xo::print::APrintable;

    namespace scm {
        bool
        SetupProcedure2::register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<ARuntimeContext, DSimpleRcx>();

            FacetRegistry::register_impl<AProcedure, DPrimitive_gco_0>();
            FacetRegistry::register_impl<AGCObject, DPrimitive_gco_0>();
            FacetRegistry::register_impl<APrintable, DPrimitive_gco_0>();

            FacetRegistry::register_impl<AProcedure, DPrimitive_gco_1_gco>();
            FacetRegistry::register_impl<AGCObject, DPrimitive_gco_1_gco>();
            FacetRegistry::register_impl<APrintable, DPrimitive_gco_1_gco>();

            FacetRegistry::register_impl<AProcedure, DPrimitive_gco_2_gco_gco>();
            FacetRegistry::register_impl<AGCObject, DPrimitive_gco_2_gco_gco>();
            FacetRegistry::register_impl<APrintable, DPrimitive_gco_2_gco_gco>();

            FacetRegistry::register_impl<AProcedure, DPrimitive_gco_2_dict_string>();
            FacetRegistry::register_impl<AGCObject, DPrimitive_gco_2_dict_string>();
            FacetRegistry::register_impl<APrintable, DPrimitive_gco_2_dict_string>();

            FacetRegistry::register_impl<AProcedure, DPrimitive_gco_3_dict_string_gco>();
            FacetRegistry::register_impl<AGCObject, DPrimitive_gco_3_dict_string_gco>();
            FacetRegistry::register_impl<APrintable, DPrimitive_gco_3_dict_string_gco>();

            log && log(xtag("DSimpleRcx.tseq", typeseq::id<DSimpleRcx>()));
            log && log(xtag("DPrimitive_gco_0.tseq", typeseq::id<DPrimitive_gco_0>()));
            log && log(xtag("DPrimitive_gco_1_gco.tseq", typeseq::id<DPrimitive_gco_1_gco>()));
            log && log(xtag("DPrimitive_gco_2_gco_gco.tseq", typeseq::id<DPrimitive_gco_2_gco_gco>()));
            log && log(xtag("DPrimitive_gco_2_dict_string.tseq", typeseq::id<DPrimitive_gco_2_dict_string>()));
            log && log(xtag("DPrimitive_gco_3_dict_string_gco.tseq", typeseq::id<DPrimitive_gco_3_dict_string_gco>()));

            log && log(xtag("ARuntimeContext.tseq", typeseq::id<ARuntimeContext>()));
            log && log(xtag("AProcedure.tseq", typeseq::id<AProcedure>()));

            return true;
        }

        bool
        SetupProcedure2::register_types(obj<ACollector> gc)
        {
            scope log(XO_DEBUG(true));

            bool ok = true;

            // (note: don't currently intend to support AGCObject for DSimpleRcx)

            ok &= gc.install_type(impl_for<AGCObject, DPrimitive_gco_0>());
            ok &= gc.install_type(impl_for<AGCObject, DPrimitive_gco_1_gco>());
            ok &= gc.install_type(impl_for<AGCObject, DPrimitive_gco_2_gco_gco>());
            ok &= gc.install_type(impl_for<AGCObject, DPrimitive_gco_2_dict_string>());
            ok &= gc.install_type(impl_for<AGCObject, DPrimitive_gco_3_dict_string_gco>());

            return ok;
        }

        bool
        SetupProcedure2::register_primitives(obj<ARuntimeContext> rcx,
                                             InstallSink sink,
                                             InstallFlags flags)
        {
            obj<AAllocator> mm = rcx.allocator();
            StringTable * stbl = rcx.stringtable();

            scope log(XO_DEBUG(false));

            bool ok = true;

            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            ObjectPrimitives::make_cwd_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));
            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            ObjectPrimitives::make_nth_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));
            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            ObjectPrimitives::make_cons_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));
            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            ObjectPrimitives::make_set_car_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));
            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            ObjectPrimitives::make_dict_make_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));
            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            ObjectPrimitives::make_dict_lookup_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));
            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            ObjectPrimitives::make_dict_upsert_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));
            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            ObjectPrimitives::make_fn_n_args_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));

            // ----- gc primitives -----

            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            GcPrimitives::make_report_gc_statistics_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));

            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            GcPrimitives::make_report_gc_object_types_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));

            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            GcPrimitives::make_report_gc_object_ages_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));

            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            GcPrimitives::make_gc_location_of_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));

            ok = ok & (PrimitiveRegistry::install_aux
                           (sink,
                            GcPrimitives::make_request_gc_pm(mm, stbl),
                            flags & InstallFlags::f_generalpurpose));

            return ok;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end SetupProcedure2.cpp */
