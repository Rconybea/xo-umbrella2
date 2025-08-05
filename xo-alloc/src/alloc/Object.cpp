/* Object.cpp
 *
 * author: Roalnd Conybeare, Jul 2025
 */

#include "Object.hpp"
#include "GC.hpp"
#include "Forwarding1.hpp"

using xo::obj::Forwarding1;

void *
operator new (std::size_t z, const xo::Cpof & cpof)
{
    using xo::gc::GC;

    GC * gc = reinterpret_cast<GC *>(xo::Object::mm);

    return gc->alloc_gc_copy(z, cpof.src_);
}

namespace xo {
    gc::IAlloc *
    Object::mm = nullptr;

    Object *
    Object::_forward(Object * src, gc::GC * gc)
    {
        if (!src)
            return src;

        if (src->_is_forwarded())
            return src->_offset_destination(src);

        bool full_move = gc->runstate().full_move();

        if (!full_move && (gc->tospace_generation_of(src) == gc::generation_result::tenured)) {
            /* don't move tenured objects during incremental collection */
            return src;
        }

        Object::_shallow_move(src, gc);

        /* *src is now a forwarding pointer to copy in to-space */

        return src->_offset_destination(src);
    }

    Object *
    Object::_deep_move(Object * from_src, gc::GC * gc, gc::ObjectStatistics * /*stats*/)
    {
        using gc::generation;

        if (!from_src)
            return nullptr;

        Object * retval = from_src->_destination();

        if (retval)
            return retval;

        bool full_move = gc->runstate().full_move();

        if (!full_move && gc->tospace_generation_of(from_src) == gc::generation_result::tenured) {
            /** incremental collection does not move already-tenured objects **/
            return from_src;
        }

        /**
         *  To-space:
         *
         *     to_lo = start of to-space
         *     w,W   = white objects. An object x is white if x + all immediate children of x are in to-space
         *             (also implies this GC cycle put it there)
         *     g,G   = grey  objects. An object x is gray if it's in to-space,
         *             but possibly has >0 black children
         *     _     = free to-space memory
         *     N     = nursery space
         *     T     = tenured space
         *
         *     wwwwwwwwwwwwwwwwwwwggggggggggggggggggggg_________________...
         *     ^                  ^                    ^
         *     to_lo              grey_lo(N)           free_ptr(N)
         *
         *  After moving children of one object, advancing {nursery_grey_lo, nursery_free_ptr}
         *
         *     wwwwwwwwwwwwwwwwwwwWWWWgggggggggggggggggGGGGGGGGGGG______...
         *     ^                      ^                           ^
         *     to_lo                  grey_lo(N)                  free_ptr(N)
         *
         *  Invariant:
         *
         *     objects in [to_lo, gray_lo) are white.
         *     all gray objects are in [gray_lo, free_ptr)
         *     memory starting at free_ptr is free.
         *
         *  deep_move terminates when gray_lo catches up to free_ptr
         *
         *  Above is simplified. Complication is that GC (including incremental) may
         *  promote objects from nursery (N) to tenured (T)
         *
         *  So more accurate before/after picture
         *
         *  N  wwwwwwwwwwwwwwwwwwwggggggggggggggggggggg_________________...
         *     ^                  ^                    ^
         *     to_lo(N)           grey_lo(N)           free_ptr(N)
         *
         *  T  wwwwwwwwwwwwwwgggggggggggg_______________________________...
         *     ^             ^           ^
         *     to_lo(T)      grey_lo(T)  free_ptr(N)
         *
         *  After moving children of one object, advancing {nursery_grey_lo, nursery_free_ptr}
         *
         *  N  wwwwwwwwwwwwwwwwwwwWWWWgggggggggggggggggGGGGGGGGGGG_____...
         *     ^                      ^                           ^
         *     to_lo(N)               grey_lo(N)                  free_ptr(N)
         *
         *  T  wwwwwwwwwwwwwwggggggggggggGGGGG_________________________...
         *     ^             ^                ^
         *     to_lo(T)      grey_lo(T)       free_ptr(T)
         *
         *  deep_move terminates when both:
         *  - gray_lo(N) catches up with free_ptr(N)
         *  - gray_lo(T) catches up with free_ptr(T)
         *
         **/

        std::array<std::byte *, gen2int(generation::N)> gray_lo_v
            = { gc->free_ptr(generation::nursery), gc->free_ptr(generation::tenured) };

        Object * to_src = Object::_shallow_move(from_src, gc);

        std::size_t fixup_work = 0;
        do {
            fixup_work = 0;

            auto fixup_generation = [gc, &gray_lo_v](generation gen) {
                std::size_t work = 0;
                while(gray_lo_v[gen2int(gen)] < gc->free_ptr(gen)) {
                    Object * x = reinterpret_cast<Object *>(gray_lo_v[gen2int(gen)]);

                    // update per-class stats here

                    std::size_t xz = x->_forward_children();

                    // must pad xz to multiple of word size,
                    // to match behavior of LinearAlloc::alloc()
                    //
                    xz += gc::IAlloc::alloc_padding(xz);

                    gray_lo_v[gen2int(gen)] += xz;
                    ++work;
                }

                return work;
            };

            fixup_work += fixup_generation(generation::nursery);
            fixup_work += fixup_generation(generation::tenured);
        } while (fixup_work > 0);

        return to_src;
    } /*deep_move*/

    Object *
    Object::_shallow_move(Object * src, gc::GC * gc)
    {
        /* filter for source objects that are owned by GC.
         * Care required though -- during GC from/to spaces have been swapped already
         */
        if (gc->fromspace_contains(src))
        {
            Object * dest = src->_shallow_copy();

            if (dest != src)
                src->_forward_to(dest);

            return dest;
        } else {
            return src;
        }
    }

    void
    Object::_forward_to(Object * dest)
    {
        char * mem = reinterpret_cast<char *>(this);

        Forwarding1 * fwd = new (mem) Forwarding1(dest);

        (void)fwd;
    }

} /*namespace xo*/

/* end Object.cpp*/
