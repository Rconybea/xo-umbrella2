/** @file ICollector_DX1Collector.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 *
 *  See also IAllocator_DX1Collector.cpp for allocator facet
 **/

#include "xo/gc/detail/ICollector_DX1Collector.hpp"
#include "GCObject.hpp"

namespace xo {
    namespace mm {
        using size_type = ICollector_DX1Collector::size_type;
        using std::byte;

        namespace {
            size_type
            stat_helper(const DX1Collector & d,
                        size_type (DArena::* getter)() const,
                        generation g,
                        role r)
            {
                const DArena * arena = d.get_space(r, g);

                if (arena) [[likely]]
                    return (arena->*getter)();

                return 0;
            }
        }

        size_type
        ICollector_DX1Collector::reserved(const DX1Collector & d, generation g, role r)
        {
            return stat_helper(d, &DArena::reserved, g, r);
        }

        size_type
        ICollector_DX1Collector::allocated(const DX1Collector & d, generation g, role r)
        {
            return stat_helper(d, &DArena::allocated, g, r);
        }

        size_type
        ICollector_DX1Collector::committed(const DX1Collector & d, generation g, role r)
        {
            return stat_helper(d, &DArena::committed, g, r);
        }

        bool
        ICollector_DX1Collector::is_type_installed(const DX1Collector & d, typeseq tseq)
        {
            return d.is_type_installed(tseq);
        }

        bool
        ICollector_DX1Collector::install_type(DX1Collector & d,
                                              const AGCObject & iface)
        {
            return d.install_type(iface);
        }

        void
        ICollector_DX1Collector::add_gc_root(DX1Collector & d,
                                             int32_t tseq,
                                             Opaque * root)
        {
            d.add_gc_root(typeseq(tseq), root);
        }

        void
        ICollector_DX1Collector::request_gc(DX1Collector & d,
                                            generation upto)
        {
            d.request_gc(upto);
        }

        void
        ICollector_DX1Collector::forward_inplace(DX1Collector & d,
                                                 AGCObject * lhs_iface,
                                                 void ** lhs_data)
        {
            (void)lhs_iface;
            assert(d.runstate_.is_running());

            /*
             *   lhs   obj<AGCObject>
             *    |    +---------+                +---+-+----+
             *    \--->| .iface  |                | T |G|size| header
             *         +---------+  object_data   +---+-+----+
             *         | .data x----------------->| alloc    |
             *         +---------+                | data     |
             *                                    | for      |
             *                                    | instance |
             *                                    | ...      |
             *                                    +----------+
             */

            void * object_data = (byte *)lhs_data;

            if (!d.contains(role::from_space(), object_data)) {
                /* *lhs isn't in GC-allocated space.
                 *
                 * This happens for a modest number of global
                 * constant, for example DBoolean {true, false}.
                 *
                 * It's important we recognize these up front.
                 * Since not allocated from GC, they don't have
                 * an alloc-header.
                 */
                return;
            }

            /** NOTE: for form's sake:
             *        better to lookup actual arena that
             *        allocated object data.
             *
             **/
            DArena * some_arena = d.to_space(generation(0));

            DArena::header_type * p_header
                = some_arena->obj2hdr(object_data);

            DArena::header_type alloc_hdr = *p_header;

            /* recover allocation size */
            std::size_t alloc_z = some_arena->config_.header_.size(alloc_hdr);

            /* need to be able to fit forwarding pointer
             * in place of forwarded object.
             *
             * This is guaranteed anyway, by alignment rules
             */
            assert(alloc_z > sizeof(uintptr_t));

            if (d.is_forwarding_header(alloc_hdr)) {
                /* *lhs already refers to a forwarding pointer */

                /*
                 *   lhs   obj<AGCObject>
                 *    |    +---------+                +---+-+----+
                 *    \--->| .iface  |                |FWD|G|size| alloc_hdr
                 *         +---------+  object_data   +---+-+----+
                 *         | .data x----------------->|     x-------->
                 *         +---------+                |          |  dest
                 *                                    |          |
                 *                                    +----------+
                 */
                void * dest = *(void**)object_data;

                /* update *lhs in-place */
                *lhs_data = dest;
            } else if (check_move_policy(d, alloc_hdr, object_data)) {
                /* copy object *lhs + replace with forwarding pointer */

                /* which arena are we writing to? need allocator interface */

                assert(false);

#ifdef NOT_YET
                // to do this need IAllocator_DX1Collector fully implemented

                void * copy = (*lhs).shallow_copy(xxx);

                    xxx mm xxx;
#endif
            } else {
                /* object doesn't need to move.
                 * e.g. incremental collection + object is tenured
                 */
            }
        }

        bool
        ICollector_DX1Collector::check_move_policy(const DX1Collector & d,
                                                   header_type alloc_hdr,
                                                   void * object_data)
        {
            (void)object_data;

            // when gc is moving objects, to- and from- spaces have been
            // reversed: forwarding pointers are located in from-space and
            // refer to to-space.

            object_age age = d.header2age(alloc_hdr);

            generation g = d.config_.age2gen(age);

            assert(d.runstate_.is_running());

            return (g < d.runstate_.gc_upto());
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end ICollector_DX1Collector.cpp */
