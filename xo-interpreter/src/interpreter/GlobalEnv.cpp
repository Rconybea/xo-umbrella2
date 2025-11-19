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
                             const rp<GlobalSymtab> & symtab) : mm_{mm}, symtab_{symtab}
        {}

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
