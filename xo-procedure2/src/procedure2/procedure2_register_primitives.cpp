/** @file procedure2_register_primitives.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "procedure2_register_primitives.hpp"
#include "ObjectPrimitives.hpp"
#include <xo/object2/Sequence.hpp>
#include <xo/object2/Integer.hpp>

namespace xo {
    using xo::scm::ASequence;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;

    namespace scm {
        template <typename PrimitiveRepr>
        bool install_aux(InstallSink sink,
                         PrimitiveRepr * pm,
                         InstallFlags flags)
        {
            scope log(XO_DEBUG(true));

            if ((flags & InstallFlags::f_generalpurpose) == InstallFlags::f_generalpurpose) {
                log && log("create primitive", xtag("name", pm->name()));

                return sink(pm->name(),
                            pm->fn_td(),
                            obj<AProcedure,PrimitiveRepr>(pm),
                            flags);
            } else {
                log && log("skip primitive", xtag("name", pm->name()));

                return true;
            }
        }

        template <typename Primitive>
        bool install_aux(InstallSink sink,
                         obj<AAllocator> mm,
                         std::string_view name,
                         typename Primitive::FunctionPtrType impl,
                         InstallFlags flags)
        {
            if (flags != InstallFlags::f_none) {
                auto pm
                    = Primitive::_make(mm, name, impl);

                return install_aux(sink, pm, flags);
            } else {
                return true;
            }
        }

        bool
        procedure2_register_primitives(obj<xo::mm::AAllocator> mm,
                                       StringTable * stbl,
                                       InstallSink sink,
                                       InstallFlags flags)
        {
            scope log(XO_DEBUG(true));

            bool ok = true;

            ok = ok & install_aux(sink, ObjectPrimitives::make_cwd_pm(mm, stbl), flags);
            ok = ok & install_aux(sink, ObjectPrimitives::make_nth_pm(mm, stbl), flags);
            ok = ok & install_aux(sink, ObjectPrimitives::make_cons_pm(mm, stbl), flags);
            ok = ok & install_aux(sink, ObjectPrimitives::make_dict_make_pm(mm, stbl), flags);
            ok = ok & install_aux(sink, ObjectPrimitives::make_dict_lookup_pm(mm), flags);
            ok = ok & install_aux(sink, ObjectPrimitives::make_dict_upsert_pm(mm), flags);
            ok = ok & install_aux(sink, ObjectPrimitives::make_fn_n_args_pm(mm), flags);

            return ok;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end procedure2_register_primitives.cpp */
