/** @file ObjectPrimitives.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "ObjectPrimitives.hpp"
#include <xo/object2/RuntimeError.hpp>
#include <xo/object2/Dictionary.hpp>
#include <xo/object2/Sequence.hpp>
#include <xo/object2/List.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/stringtable2/String.hpp>
#include <unistd.h> // for getcwd()

namespace xo {
    using xo::scm::ASequence;
    using xo::print::APrintable;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::facet::FacetRegistry;
    using xo::facet::TypeRegistry;

    namespace scm {

        // ----- cwd -----

        obj<AGCObject>
        xfer_cwd(obj<ARuntimeContext> rcx)
        {
            char buf[PATH_MAX];
            ::getcwd(buf, sizeof(buf));

            return obj<AGCObject,DString>(DString::from_cstr(rcx.allocator(), buf));
        }

        DPrimitive_gco_0 *
        ObjectPrimitives::make_cwd_pm(obj<AAllocator> mm)
        {
            return DPrimitive_gco_0::_make(mm, "cwd", &xfer_cwd);
        }

        // ----- nth -----

        // TODO: seq_gc -> obj<ASequence>
        //       n_gco -> obj<AGCObject,DInteger>
        //
        obj<AGCObject>
        xfer_nth(obj<ARuntimeContext> rcx,
                 obj<AGCObject> seq_gco,
                 obj<AGCObject> n_gco)
        {
            (void)rcx;

            obj<ASequence> seq = seq_gco.to_facet<ASequence>();
            auto n = obj<AGCObject,DInteger>::from(n_gco);

            return seq.at(n->value());
        }

        DPrimitive_gco_2_gco_gco *
        ObjectPrimitives::make_nth_pm(obj<AAllocator> mm)
        {
            return DPrimitive_gco_2_gco_gco::_make(mm, "nth", &xfer_nth);
        }

        // ----- cons -----

        obj<AGCObject>
        xfer_cons(obj<ARuntimeContext> rcx,
                  obj<AGCObject> car,
                  obj<AGCObject> cdr)
        {
            (void)rcx;

            auto cdr_list = obj<AGCObject,DList>::from(cdr);

            return DList::cons(rcx.allocator(),
                               car,
                               cdr_list.data());
        }

        DPrimitive_gco_2_gco_gco *
        ObjectPrimitives::make_cons_pm(obj<AAllocator> mm)
        {
            return DPrimitive_gco_2_gco_gco::_make(mm, "cons", &xfer_cons);
        }

        // ----- dict_make -----

        obj<AGCObject>
        xfer_dict_make(obj<ARuntimeContext> rcx)
        {
            return obj<AGCObject,DDictionary>(DDictionary::empty(rcx.allocator(),
                                                                 8 /*cap*/));
        }

        DPrimitive_gco_0 *
        ObjectPrimitives::make_dict_make_pm(obj<AAllocator> mm)
        {
            return DPrimitive_gco_0::_make(mm, "dict_make", &xfer_dict_make);
        }

        // ----- dict_at -----

        obj<AGCObject>
        xfer_dict_lookup(obj<ARuntimeContext> rcx,
                         obj<AGCObject,DDictionary> dict,
                         obj<AGCObject,DString> key)
        {
            auto opt = dict->lookup(key.data());

            if (opt) {
                return opt.value();
            } else {
                DString * src_fn = DString::from_cstr(rcx.allocator(), "dict_lookup");
                DString * error = DString::printf(rcx.allocator(),
                                                  100,
                                                  "no value in dict for key [%s]", key.data()->data());

                return obj<AGCObject,DRuntimeError>
                    (DRuntimeError::_make(rcx.allocator(),
                                          src_fn, error));
            }
        }

        DPrimitive_gco_2_dict_string *
        ObjectPrimitives::make_dict_lookup_pm(obj<AAllocator> mm)
        {
            return DPrimitive_gco_2_dict_string::_make(mm, "dict_lookup", &xfer_dict_lookup);
        }

        // ----- dict_upsert -----

        obj<AGCObject>
        xfer_dict_upsert(obj<ARuntimeContext> rcx,
                         obj<AGCObject,DDictionary> dict,
                         obj<AGCObject,DString> key,
                         obj<AGCObject> value)
        {
            scope log(XO_DEBUG(true));

            log && log(xtag("dict.tseq", dict._typeseq()),
                       xtag("dict.tname", TypeRegistry::id2name(dict._typeseq())));
            log && log(xtag("key.tseq", key._typeseq()),
                       xtag("key.tname", TypeRegistry::id2name(key._typeseq())));
            log && log(xtag("value.tseq", value._typeseq()),
                       xtag("value.tname", TypeRegistry::id2name(value._typeseq())));

            auto value_pr = FacetRegistry::instance().variant<APrintable>(value);

            log && log(xtag("value", value_pr));

            dict->upsert(rcx.allocator(),
                         DDictionary::pair_type(key.data(), value));

            return dict;
        }

        DPrimitive_gco_3_dict_string_gco *
        ObjectPrimitives::make_dict_upsert_pm(obj<AAllocator> mm)
        {
            return DPrimitive_gco_3_dict_string_gco::_make(mm, "dict_upsert", &xfer_dict_upsert);
        }

        // ----- fn_n_args -----

        obj<AGCObject>
        xfer_fn_n_args(obj<ARuntimeContext> rcx,
                       obj<AGCObject> fn_gco)
        {
            scope log(XO_DEBUG(true));

            log && log(xtag("fn_gco.tseq", fn_gco._typeseq()));
            log && log(xtag("fn_gco.tname", TypeRegistry::id2name(fn_gco._typeseq())));

            auto fn_proc = FacetRegistry::instance().try_variant<AProcedure,AGCObject>(fn_gco);

            assert(fn_proc);

            return DInteger::box<AGCObject>(rcx.allocator(), fn_proc.n_args());
        }

        DPrimitive_gco_1_gco *
        ObjectPrimitives::make_fn_n_args_pm(obj<AAllocator> mm)
        {
            return DPrimitive_gco_1_gco::_make(mm, "fn_n_args", &xfer_fn_n_args);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end ObjectPrimitives.cpp */
