/* Object.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include <xo/reflect/TaggedPtr.hpp>
#include <xo/allocutil/IObject.hpp>
#include <xo/allocutil/ObjectVisitor.hpp>
#include <xo/allocutil/gc_ptr.hpp>
/* Prettifier.hpp -> PpSink.hpp, which is deliberately ostream-free: it costs
 * <iosfwd> <string_view> <cstddef> <cstdint> <optional> and nothing more.
 * Object.hpp sits at the root of the object model, so that weight is the
 * reason PpSink is preferred here over the printing stack proper.
 */
#include <xo/ppsink/Prettifier.hpp>
#include <concepts>
#include <cstdint>

namespace xo {
    namespace gc {
        class IAlloc;
        class GC;
        class ObjectStatistics;
    };

    /** Root class for all xo GC-collectable objects.
     *
     *  Design notes:
     *  1. xo::IObject -> xo-allocutil  header-only library
     *     xo::Object  -> xo-alloc      ordinary library
     *  2. relying on inheritance means we insist that GC traits
     *     for a type appear directly in that type's vtable, and at specific locations.
     *     This implies one level of indirection when GC traverses an instance.
     *  3. Could adapt a gc-aware XO library (such as xo-ordinaltree)
     *     to a non-xo garbage collector.
     *     Would still need to use xo::IObject and xo::gc::gc_allocator_traits,
     *     but not necessarily xo::Object
     *  4. Would be feasible to relax the must-inherit-from-Object constraint
     *     by having GC use its own wrapper, at cost of an extra layer of indirection
     **/
    class Object : public IObject {
    public:
        using TaggedPtr = xo::reflect::TaggedPtr;

    public:
        static gp<Object> from(gp<IObject> x) {
            return dynamic_cast<Object*>(x.ptr());
        }

        virtual ~Object() noexcept = default;

        /** memory allocator for objects.  Likely this will be a GC instance,
         *  but simple arena also supported.
         *
         *  Load-bearing for .assign_member()
         **/
        static gc::IAlloc * mm;

        /** assign value @p rhs to member @p *lhs of @p parent.
         *  if assignment creates a cross-generational or cross-checkpoint pointer,
         *  add mutation log entry.
         *
         *  DEPRECATED. prefer IObject::_gc_assign_member, for explicit alloc
         **/
        template <typename T>
        static void assign_member(gp<IObject> parent, gp<T> * lhs, gp<IObject> rhs);

        /** use from GC aux functions **/
        static gc::GC * _gc() { return reinterpret_cast<gc::GC*>(mm); }

        /** during GC
         *  1. copy destination object @p *addr to (new) to-space.
         *  2. overwrite existing object @p *addr with a forwarding pointer to
         *     copy made in step 1.
         *  3. return the location of the copy make in step 1.
         *
         *  @p src.        source object to be forwarded
         *  @p gc.         allocator (poassibly garbage collector)
         */
        static IObject * _forward(IObject * src, gc::IAlloc * gc);

        template <typename T>
        static void _forward_inplace(T ** src_addr, gc::IAlloc * gc) {
            IObject * fwd = _forward(*src_addr, gc);

            *src_addr = reinterpret_cast<T *>(fwd);
        }

        template <typename T>
        static void _forward_inplace(gp<T> & src, gc::IAlloc * gc) {
            _forward_inplace<T>(src.ptr_address(), gc);
        }

        /** primary workhorse for garbage collection.
         *
         *  we assign each object one of three colors: black|gray|white.
         *
         *  color | location   | children   | action                  |
         *  ------+------------+------------+-------------------------+
         *  black | from-space | any        | move to to-space        |
         *  gray  |   to-space | any        | move remaining children |
         *  white |   to-space | white/gray | done                    |
         *
         *  initially all reachable objects are black.
         *  GC is complete when all reachable objects are white.
         *  GC needs a variable amount of temporary storage to keep track of all gray objects
         *
         *  Evacuate reachable object graph rooted at @p src to to-space.
         *  On return all objects reachable from @p src are white
         *
         *  @param src     address of object to evacuate
         *  @param gc      garbage collector
         *  @param stats   per-object-type GC statistics
         **/
        static IObject * _deep_move(IObject * src, gc::GC * gc, gc::ObjectStatistics * stats);

        /** copy @p src to to-space. Overwrite original with forwarding pointer to new location.
         *  return the new location
         **/
        static IObject * _shallow_move(IObject * src, gc::IAlloc * gc);

        // Reflection support

        /** tagged pointer with runtime type information
         **/
        virtual TaggedPtr self_tp() const;

        /** structured pretty-printing: render this object into @p sink.
         *
         *  This replaced a virtual display(std::ostream&) -- see
         *  .xo-backlog/xo-alloc/issues/01-object-pretty-ppsink.md.  A PpSink
         *  nests: an object rendered as part of an enclosing structure
         *  participates in that structure's line breaking, where an ostream
         *  inserter flattened it into one unbreakable token.
         *
         *  Object's own definition renders "<Object>", so a subclass that
         *  overrides nothing still prints something identifiable.
         *
         *  For a std::ostream, include xo/alloc/alloc_ostream.hpp.
         **/
        virtual void pretty(xo::pp::PpSink & sink) const;

        // Inherited from IObject..

        //virtual bool _is_forwarded() const override { return false; }
        //virtual IObject * _offset_destination(IObject * src) const override { return src; };
        virtual void _forward_to(IObject * dest) override;
        //virtual IObject * _destination() override { return nullptr; }

        virtual std::size_t _shallow_size() const override = 0;
        virtual IObject * _shallow_copy(gc::IAlloc * gc) const override = 0;
        virtual std::size_t _forward_children(gc::IAlloc * gc) override = 0;
    };

    static_assert(std::is_destructible_v<Object>, "Object must be destructible");
    static_assert(std::is_nothrow_destructible_v<Object>, "Object must be noexcept destructible");

    template <typename T>
    void
    Object::assign_member(gp<IObject> parent, gp<T> * lhs, gp<IObject> rhs)
    {
        Object::mm->assign_member(reinterpret_cast<IObject *>(parent.ptr()),
                                  reinterpret_cast<IObject **>(lhs->ptr_address()),
                                  reinterpret_cast<IObject *>(rhs.ptr()));
    }

    namespace gc {
        template <typename T>
        class ObjectVisitor<gp<T>> {
        public:
            static void forward_children(gp<T> & target,
                                         IAlloc * gc)
            {
                Object::_forward_inplace(target, gc);
            }
        };
    }

} /*namespace xo*/

namespace xo::pp {
    /** pretty-print a gp<T> into a PpSink, for any T in the Object hierarchy.
     *
     *  Without this, gp<T> falls through Prettifier's empty primary template
     *  to operator<<, which flattens the whole object into one string token --
     *  so a nested object could not break, however narrow the margin.  With
     *  it, xtag("k", obj) / field("k", obj) / sink.pp(obj) all render the
     *  object structurally.  Pinned by
     *  xo-alloc/utest/object_pretty.test.cpp.
     *
     *  NB constrained on the whole hierarchy, not written as an exact
     *  Prettifier<gp<Object>>.  The inserter this replaced took gp<Object> and
     *  accepted a gp<Derived> by implicit conversion; template argument
     *  matching grants no such conversion, so an exact specialization would
     *  silently miss every derived type and send it back to operator<<.
     **/
    template <typename T>
        requires std::derived_from<T, xo::Object>
    struct Prettifier<xo::gp<T>> {
        static void print(PpSink & sink, const xo::gp<T> & x) {
            if (x.ptr())
                x->pretty(sink);
            else
                sink.pp("<nullptr>");
        }
    };
} /*namespace xo::pp*/

void * operator new (std::size_t z, const xo::Cpof & copy);

/* end Object.hpp */
