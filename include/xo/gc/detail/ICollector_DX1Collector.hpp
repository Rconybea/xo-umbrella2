/** @file ICollector_DX1Collector.hpp
*
 *  @author Roland Conybeare, Dec 2025
 **/

#include "ACollector.hpp"
#include "ICollector_Xfer.hpp"
#include "DX1Collector.hpp"

namespace xo {
    namespace mm { struct ICollector_DX1Collector; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::ACollector,
                                   xo::mm::DX1Collector>
        {
            using ImplType = xo::mm::ICollector_Xfer<xo::mm::DX1Collector,
                                                     xo::mm::ICollector_DX1Collector>;
        };
    }

    namespace mm {
        /* changes here coordinate with
         *  ACollector      ACollector.hpp
         *  ICollector_Any  ICollector_Any.hpp
         *  ICollector_Xfer ICollector_Xfer.hpp
         *  RCollector      RCollector.hpp
         */
        struct ICollector_DX1Collector {
            using size_type = std::size_t;
            using header_type = DArena::header_type;
            using typeseq = xo::facet::typeseq;

            static bool check_move_policy(const DX1Collector & d,
                                          header_type alloc_hdr,
                                          void * object_data);

            // todo: available()

            static size_type allocated(const DX1Collector & d, generation g, role r);
            static size_type reserved(const DX1Collector & d, generation g, role r);
            static size_type committed(const DX1Collector & d, generation g, role r);
            static bool is_type_installed(const DX1Collector & d, typeseq tseq);

            static bool install_type(DX1Collector & d, const AGCObject & iface);
            static void add_gc_root_poly(DX1Collector & d, obj<AGCObject> * p_root);
            static void request_gc(DX1Collector & d, generation upto);
            static void forward_inplace(DX1Collector & d, AGCObject * lhs_iface, void ** lhs_data);

            static int32_t s_typeseq;
            static bool _valid;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end ICollector_DX1_Collector.hpp */
