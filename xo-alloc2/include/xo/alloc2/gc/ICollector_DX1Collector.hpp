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
        struct FacetImplementation<xo::mm::ACollector, xo::mm::DX1Collector> {
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

            static size_type allocated(const DX1Collector & d, generation g, role r) {
                const DArena * arena = d.get_space(r, g);

                if (arena) [[likely]] {
                    return arena->allocated();
                } else {
                    return 0;
                }
            }

            static int32_t s_typeseq;
            static bool _valid;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end ICollector_DX1_Collector.hpp */
