/** @file IAllocator_DArena.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AAllocator.hpp"
#include "IAllocator_Xfer.hpp"
#include "DArena.hpp"

namespace xo {
    namespace mm {
        struct IAllocator_DArena;
    }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AAllocator, xo::mm::DArena> {
            using ImplType = xo::mm::IAllocator_Xfer<xo::mm::DArena,
                                                     xo::mm::IAllocator_DArena>;
        };
    }

    namespace mm {
        struct IAllocator_DArena {
            static const std::string & name(const DArena &);
            static std::size_t reserved(const DArena &);
            static std::size_t size(const DArena &);
            static std::size_t committed(const DArena &);
            static bool contains(const DArena &, const void * p);
            static std::byte * alloc(const DArena &, std::size_t z);
            static void clear(DArena &);
            static void destruct_data(DArena &);
        };

//        template <>
//        struct IAllocator_Impl<DArena> {
//            using ImplType = IAllocator_DArena;
//        };

    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_DArena.hpp */
