/** @file DLocalSymtab.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DUniqueString.hpp"
#include "LocalSymtab.hpp"
#include "Typename.hpp"
#include "Variable.hpp"
#include <xo/object2/Array.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/ppsink/concat.hpp>         /* concat("[", i, "]") for generated names */
#include <xo/ppsink/pretty_struct.hpp>  /* sink.struct_open(..), struct_scope */

namespace xo {
    using xo::mm::ACollector;
    using xo::mm::AGCObject;
    using xo::print::APrintable;

    namespace scm {

        DLocalSymtab::DLocalSymtab(DLocalSymtab * p,
                                   DArray * vars, DArray * types)
            : parent_{p}, vars_{vars}, types_{types}
        {
        }

        DLocalSymtab *
        DLocalSymtab::_make_empty(obj<AAllocator> mm,
                                  DLocalSymtab * p,
                                  size_type nv,
                                  size_type nt)
        {
            void * mem = mm.alloc_for<DLocalSymtab>();

            DArray * vars = DArray::_empty(mm, nv);
            DArray * types = DArray::_empty(mm, nt);

            return new (mem) DLocalSymtab(p, vars, types);
        }

        DVariable *
        DLocalSymtab::lookup_var(Binding ix) noexcept
        {
            assert(ix.i_link() == 0);
            assert(ix.j_slot() < static_cast<int32_t>(vars_->size()));

            auto var = obj<AGCObject,DVariable>::from((*vars_)[ix.j_slot()]);

            assert(var);

            return var.data();
        }

        Binding
        DLocalSymtab::append_var(obj<AAllocator> mm,
                                 const DUniqueString * name,
                                 TypeRef typeref)
        {
            assert(name);

            if (vars_->size() >= vars_->capacity() || !name) {
                assert(false);

                return Binding::null();
            } else {
                //size_type i_slot = (this->size_)++;
                Binding binding = Binding::local(vars_->size());

                DVariable * var = DVariable::make(mm, name, typeref, binding);

                vars_->push_back(mm, obj<AGCObject,DVariable>(var));

                return binding;
            }
        }

        void
        DLocalSymtab::append_type(obj<AAllocator> mm,
                                  const DUniqueString * name,
                                  obj<AType> type)
        {
            assert(name);

            if (types_->size() >= types_->capacity() || !name) {
                assert(false);
            } else {
                obj<AGCObject> tname = DTypename::make(mm, name, type);

                types_->push_back(mm, tname);
            }
        }

        Binding
        DLocalSymtab::lookup_binding(const DUniqueString * sym) const noexcept
        {
            assert(sym);

            if (sym) {
                for (size_type i = 0, n = vars_->size(); i < n; ++i) {
                    auto var_i = obj<AGCObject,DVariable>::from((*vars_)[i]);

                    assert(var_i);

                    if (*sym == *(var_i->name()))
                        return var_i->path();
                }
            }

            return Binding();
        }

        // ----- gcobject facet -----

        DLocalSymtab *
        DLocalSymtab::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DLocalSymtab::visit_gco_children(VisitReason reason,
                                         obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &parent_);
            gc.visit_child(reason, &vars_);
            gc.visit_child(reason, &types_);
        }

        // ----- printable facet -----

        void
        DLocalSymtab::pretty(xo::pp::PpSink & sink) const
        {
            /* struct_open(), not pretty_struct(): the field count is
             * 2 + n_vars + n_types, a runtime value.  Two dynamic-arity loops,
             * unlike DApplyExpr's one.
             *
             * NB struct_scope::field() renders IMMEDIATELY, so unlike the free
             * xo::pp::field() it does not capture by reference -- the size()
             * temporaries and the concat below are safe here.
             */
            auto st = sink.struct_open("LocalSymtab");

            st.field("nvars", vars_->size());

            for (size_type i = 0, n = vars_->size(); i < n; ++i) {
                obj<APrintable> var_i = (*vars_)[i].to_facet<APrintable>();

                st.field(xo::pp::concat("[", i, "]"), var_i);
            }

            st.field("ntypes", types_->size());

            /* The index names COLLIDE with the :vars loop above -- a symtab
             * with both renders :[0] twice.  Legacy did the same (snprintf
             * "[%u]" in both loops), and this conversion reproduces it rather
             * than fixing it; see the ticket.
             *
             * to_facet<APrintable> here is on a DTypename, which HAS the
             * facet.  What throws for a non-empty types_ is one level down --
             * DTypename's own type_.to_facet<APrintable>(), since xo-type's
             * D-types have no APrintable.  Confirmed by test, not code-read;
             * see .xo-backlog/xo-type/issues/01-no-aprintable-facet.md
             */
            for (size_type i = 0, n = types_->size(); i < n; ++i) {
                obj<APrintable> type_i = (*types_)[i].to_facet<APrintable>();

                st.field(xo::pp::concat("[", i, "]"), type_i);
            }
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end DLocalSymtab.cpp */
