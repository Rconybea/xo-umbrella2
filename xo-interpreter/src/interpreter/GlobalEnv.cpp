/** @file GlobalEnv.cpp **/

#include "GlobalEnv.hpp"
#include "xo/reflect/Reflect.hpp"

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;

    namespace scm {
        gp<GlobalEnv>
        GlobalEnv::make_empty(gc::IAlloc * mm, const rp<GlobalSymtab> & symtab)
        {
            /* by design: GlobalEnv and GlobalEnv.slot_map_ are heap-allocated */

             return new GlobalEnv(mm, symtab);
        }

        GlobalEnv::GlobalEnv(const GlobalEnv & x)
            : mm_{x.mm_},
              symtab_{x.symtab_},
              slot_map_{std::make_unique<map_type>(*x.slot_map_)}
        {
        }

        GlobalEnv::GlobalEnv(gc::IAlloc * mm,
                             const rp<GlobalSymtab> & symtab) : mm_{mm},
                                                                symtab_{symtab},
                                                                slot_map_{std::make_unique<map_type>()}
        {}

        bool
        GlobalEnv::local_contains_var(const std::string & vname) const
        {
            return symtab_->lookup_local(vname).get();
        }

        void
        GlobalEnv::establish_var(bp<Variable> var)
        {
            // Warning: altering declared type for an already-existing variable
            // invalidates any type checking that relied on that variable.
            //
            // Ignoring this problem for now.
            //
            // Actual solution might look like:
            // - keep track of which functions/defs depend on each global variable.
            // - invalidate any jit / types for such variables.
            // - maybe use seqno's to track
            // - re-check / re-complie
            // - need to admit invalid states.
            //   suppose have mutually recursive functions f(), g()
            //   want ability to modify type signatures separately
            //
            // Alternatives:
            // - forbid changing type of an already-established variable
            //   Actually: can't even change values if we intend supporting dependent types
            // - quietly number variables so new definitions shadow old ones but don't
            //   affect previously-encountered expressions

            this->symtab_->require_global(var->name(), var);

            (*this->slot_map_)[var->name()] = gp<Object>();
        }

        TaggedPtr
        GlobalEnv::self_tp() const
        {
            return Reflect::make_tp(const_cast<GlobalEnv *>(this));
        }

        void
        GlobalEnv::display(std::ostream & os) const
        {
            os << "<global-env" << xtag("n", slot_map_->size()) << ">";
        }

        std::size_t
        GlobalEnv::_shallow_size() const
        {
            return sizeof(GlobalEnv);
        }

        Object *
        GlobalEnv::_shallow_copy(gc::IAlloc * mm) const
        {
            Cpof cpof(mm, this);

            return new (cpof) GlobalEnv(*this);
        }

        std::size_t
        GlobalEnv::_forward_children(gc::IAlloc * gc)
        {
            for (auto & ix : *slot_map_) {
                Object::_forward_inplace(ix.second, gc);
            }
            return _shallow_size();
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end GlobalEnv.cpp */
