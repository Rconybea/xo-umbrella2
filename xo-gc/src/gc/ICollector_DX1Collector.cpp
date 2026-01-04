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
        ICollector_DX1Collector::add_gc_root_poly(DX1Collector & d,
                                                  obj<AGCObject> * p_root)
        {
            d.add_gc_root_poly(p_root);
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
            d.forward_inplace(lhs_iface, lhs_data);
        }

        bool
        ICollector_DX1Collector::check_move_policy(const DX1Collector & d,
                                                   header_type alloc_hdr,
                                                   void * object_data)
        {
            return d.check_move_policy(alloc_hdr, object_data);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end ICollector_DX1Collector.cpp */
