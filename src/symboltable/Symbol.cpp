/** @file Symbol.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "Symbol.hpp"
#include "xo/reflect/Reflect.hpp"

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;

    namespace scm {

        gp<Symbol>
        Symbol::make(gc::IAlloc * mm, const char * n)
        {
            return new (MMPtr(mm)) Symbol(mm, n);
        }

        Symbol::Symbol(gc::IAlloc * mm,
                       const char * name)
        {
            std::size_t nz = ::strlen(name) + 1;
            assert(nz > 0);
            assert(nz < 4096);

            char * mem = reinterpret_cast<char *>(mm->alloc(nz));
            strncpy(mem, name, nz);

            this->name_ = mem;
            this->name_z_ = nz;
        }

        // ------ Reflection helpers -----

        TaggedPtr
        Symbol::self_tp() const
        {
            return Reflect::make_tp(const_cast<Symbol *>(this));
        }

        void
        Symbol::display(std::ostream & os) const
        {
            os << name_;
        }

        // ------ GC helpers -----

        std::size_t
        Symbol::_shallow_size() const
        {
            std::size_t retval = sizeof(Symbol);

            /* note: to remove all doubt:
             *     in case ::strlen(name_) != name_z,
             *     compute string length here.
             *
             *     Alternative would be to pass length to ctor
             *     + ensure that's what gets allocated
             */
            retval += gc::IAlloc::with_padding(::strlen(name_) + 1);

            return retval;
        }

        Object *
        Symbol::_shallow_copy() const
        {
            Cpof cpof(Object::mm, this);

            Symbol * copy = new (cpof) Symbol(cpof.mm_, name_);

            return copy;
        }

        std::size_t
        Symbol::_forward_children()
        {
            /* note: zero embedded gp<T> pointers to fixup */

            return _shallow_size();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end Symbol.cpp */
