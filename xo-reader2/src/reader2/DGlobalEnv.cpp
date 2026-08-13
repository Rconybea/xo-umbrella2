/** @file DGlobalEnv.cpp
 *
 *  @author Roland Conybeare, Feb 2026
**/

#include "GlobalEnv.hpp"
#include <xo/expression2/GlobalSymtab.hpp>
#include <xo/object2/Array.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::pp::scope;
    using xo::pp::xtag;
    using xo::pp::field;
    using xo::mm::ACollector;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;

    namespace scm {

        DGlobalEnv::DGlobalEnv(DGlobalSymtab * symtab, DArray * values)
        : symtab_{symtab}, values_{values}
        {}

        DGlobalEnv *
        DGlobalEnv::_make(obj<AAllocator> mm,
                          DGlobalSymtab * symtab)
        {
            DArray * values = DArray::_empty(mm, symtab->var_capacity());

            void * mem = mm.alloc_for<DGlobalSymtab>();

            return new (mem) DGlobalEnv(symtab, values);
        }

        obj<AGCObject>
        DGlobalEnv::lookup_value(Binding ix) const noexcept
        {
            if (!ix.is_global()) {
                assert(false);
                return obj<AGCObject>();
            }

            if (ix.j_slot() >= static_cast<int32_t>(values_->size())) {
                assert(false);
                return obj<AGCObject>();
            }

            return (*values_)[ix.j_slot()];
        }

        void
        DGlobalEnv::assign_value(obj<AAllocator> mm, Binding ix, obj<AGCObject> x)
        {
            scope log(XO_DEBUG_(false),
                      xtag("ix.j_slot", ix.j_slot()),
                      xtag("values.cap", values_->capacity()));

            assert(ix.is_global());

            if (ix.j_slot() >= static_cast<int32_t>(values_->size())) {
                // Control will come here in interpreter as new definitions are introduced.
                // After seeing
                //   def foo = 1.2345;
                // introducing new symbol foo:
                // GlobalSymtab extends to include foo without this GlobalEnv
                // knowing about it.

                if (ix.j_slot() + 1 > static_cast<int32_t>(values_->capacity())) {
                    // realloc global array for more size

                    size_t cap_2x = 2 * values_->capacity();

                    while (cap_2x < static_cast<size_t>(ix.j_slot() + 1))
                        cap_2x = 2 * cap_2x;

                    DArray * values_2x = DArray::copy(mm, values_, cap_2x);
                    assert(values_2x);

                    if (values_2x) {
                        log && log("STUB: need write barrier for GC (also in GlobalSymtab!)");
                        this->values_ = values_2x;
                    } else {
                        return;
                    }
                }

                /** expand size sot that j_slot is valid **/
                values_->resize(ix.j_slot() + 1);
            }

            values_->assign_at(mm,
                               ix.j_slot(),
                               x);
        }

        DVariable *
        DGlobalEnv::_upsert_value(obj<AAllocator> mm,
                                  const DUniqueString * sym,
                                  TypeDescr td,
                                  obj<AGCObject> value)
        {
            DVariable * var
                = DVariable::make(mm, sym, TypeRef::resolved(td));

            assert(var);

            symtab_->upsert_variable(mm, var);
            this->assign_value(mm, var->path(), value);

            return var;
        }

        // ----- AGCObject facet -----

        DGlobalEnv *
        DGlobalEnv::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for<DGlobalEnv>(this);
        }

        void
        DGlobalEnv::visit_gco_children(VisitReason reason,
                                       obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &symtab_);
            gc.visit_child(reason, &values_);
        }

        // ----- APrintable facet -----

        void
        DGlobalEnv::pretty(xo::pp::PpSink & sink) const
        {
            const auto n_vars = symtab_->n_vars();

            sink.pretty_struct("DGlobalEnv",
                               field("n_vars", n_vars));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DGlobalEnv.cpp */
