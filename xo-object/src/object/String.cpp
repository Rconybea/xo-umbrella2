/* @file String.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "String.hpp"
#include "GC.hpp"
#include "TaggedPtr.hpp"
#include "xo/reflect/Reflect.hpp"
#include <bsd/string.h>
#include <cstddef>
#include <cstring>
#include <cassert>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;

    namespace obj {
        String::String(owner owner, std::size_t z, char * s)
            : owner_{owner}, z_chars_{z}, chars_{s}
        {}

        String::String(gc::IAlloc * mm, owner owner, std::size_t z, char * s)
            : owner_{owner}, z_chars_{z}
        {
            if (owner_ == owner::unique) {
                chars_ = reinterpret_cast<char *>(mm->alloc(z));

                assert(chars_);

                strlcpy(chars_, s, z);
            } else {
                chars_ = s;
            }
        }

        gp<String>
        String::from(gp<Object> x) {
            return dynamic_cast<String*>(x.ptr());
        }

        gp<String>
        String::share(const char * s) {
            const char * chars = s ? s : "";
            std::size_t z = 1 + ::strlen(chars);

            return new (MMPtr(mm)) String(mm, owner::shared, z, const_cast<char *>(chars));
        }

        gp<String>
        String::copy(const char * s) {
            return copy(Object::mm, s);
        }

        gp<String>
        String::copy(gc::IAlloc * mm, const char * s)
        {
            std::size_t z = 1 + (s ? ::strlen(s) : 0);
            const char * chars = s ? s : "";

            // const-cast ok since chars copied with Owner::unique
            return new (MMPtr(mm)) String(mm, owner::unique, z, const_cast<char *>(chars));
        }

        gp<String>
        String::allocate(std::size_t z)
        {
            return new (MMPtr(Object::mm)) String(mm, owner::unique, z, const_cast<char *>(""));
        }

        gp<String>
        String::append(gp<String> s1, gp<String> s2)
        {
            std::size_t z1 = s1->length();
            std::size_t z2 = s2->length();
            std::size_t z = z1 + z2;

            // +1 for null terminator
            gp<String> retval = allocate(z+1);

            strlcpy(retval->chars_, s1->chars_, z1+1);
            strlcpy(retval->chars_ + z1, s2->chars_, z2+1);

            return retval;
        }

        std::size_t
        String::length() const
        {
            return ::strlen(chars_);
        }

        TaggedPtr
        String::self_tp() const {
            return Reflect::make_tp(const_cast<String*>(this));
        }

        void
        String::display(std::ostream & os) const {
            // TODO: print with escapes

            os << "\"" << c_str() << "\"";
        }

        // ----- GC support -----

        std::size_t
        String::_shallow_size() const
        {
            /* no child Object* pointers to fixup,
             * but must count for amount of storage used by _shallow_move()
             */
            std::size_t retval = gc::IAlloc::with_padding(sizeof(String));

            if (owner_ == owner::unique)
                retval += gc::IAlloc::with_padding(z_chars_);

            return retval;
        }

        Object *
        String::_shallow_copy() const
        {
            // Reminder: String must come before secondary allocation,

            Cpof cpof(this);

            // might expect to write:
            //  gp<String> copy = new (gcm) String(Object::mm, owner_, z_chars_, chars_);
            // but this would always put string contents in nursery to-space.
            //
            // We need to choose nursery/tenured based on location of this,
            // achieved by calling GC::alloc_copy() instead of GC::alloc()
            //
            gp<String> copy = new (cpof) String(owner_, z_chars_, chars_);

            if (owner_ == owner::unique) {
                std::byte * mem = reinterpret_cast<std::byte *>(chars_);

                copy->chars_ = reinterpret_cast<char *>(Object::mm->alloc_gc_copy(z_chars_, mem));
                strlcpy(copy->chars_, chars_, z_chars_);
            }

            return copy.ptr();
        }

        std::size_t
        String::_forward_children()
        {
            return this->_shallow_size();
        }

    } /*namespace obj*/
} /*namespace xo*/

/* end String.cpp */
