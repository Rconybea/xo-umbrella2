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

        GlobalEnv::GlobalEnv(gc::IAlloc * mm,
                             const rp<GlobalSymtab> & symtab) : mm_{mm},
                                                                symtab_{symtab}
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

            this->slot_map_[var->name()] = gp<Object>();
        }

        TaggedPtr
        GlobalEnv::self_tp() const
        {
            return Reflect::make_tp(const_cast<GlobalEnv *>(this));
        }

        void
        GlobalEnv::display(std::ostream & os) const
        {
            os << "<global-env" << xtag("n", slot_map_.size()) << ">";
        }

        std::size_t
        GlobalEnv::_shallow_size() const
        {
            /** 0: since not allocated in gc-space */
            return 0;
        }

        Object *
        GlobalEnv::_shallow_copy() const
        {
            /* by design, don't copy; not subject to GC */
            return const_cast<GlobalEnv *>(this);
        }

        std::size_t
        GlobalEnv::_forward_children()
        {
            /* All global slots are treated as GC roots; this means we
             * don't have to forward them
             *
             * This works only as long as global env is immortal.
             */
            return _shallow_size();
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end GlobalEnv.cpp */
