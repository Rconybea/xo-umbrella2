/** @file IAllocator_DArena.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AAllocator.hpp"
#include "IAllocator_Xfer.hpp"
#include "arena/DArena.hpp"

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
        /* changes here coordinate with:
         *  AAllocator      AAllocator.hpp
         *  IAllocator_Any  IAllocator_Any.hpp
         *  IAllocator_Xfer IAllocator_Xfer.hpp
         *  RAllocator      RAllocator.hpp
         */
        struct IAllocator_DArena {
            using size_type = std::size_t;
            using value_type = std::byte *;

            enum class alloc_mode : uint8_t {
                standard,
                super,
                sub_incomplete,
                sub_complete,
            };

            static std::string_view name(const DArena &) noexcept;
            static size_type reserved(const DArena &) noexcept;
            static size_type size(const DArena &) noexcept;
            static size_type committed(const DArena &) noexcept;
            static size_type available(const DArena &) noexcept;
            static size_type allocated(const DArena &) noexcept;
            static bool contains(const DArena &, const void * p) noexcept;
            static AllocatorError last_error(const DArena &) noexcept;

            /** expand committed space in arena @p d
             *  to size at least @p z
             *  In practice will round up to a multiple of @ref page_z_.
             **/
            static bool expand(DArena & d, size_type z) noexcept;

            static value_type alloc(DArena &, size_type z);
            /** when store_header_flag enabled:
             *   like alloc(), but combine memory consumed by this alloc
             *   plus following consecutive sub_alloc()'s into a single header.
             *  otherwise equivalent to alloc()
             **/
            static value_type super_alloc(DArena &, size_type z);
            /** when store_header_flag enabled:
             *  follow preceding super_alloc() by one or more sub_allocs().
             *  accumulate total allocated size (including padding) into
             *  single header. All sub_allocs() except the last must set
             *  @p complete_flag to false. The last sub_alloc() must set
             *  @p complete_flag to true.
             **/
            static value_type sub_alloc(DArena &, size_type z, bool complete_flag);
            static void clear(DArena &);
            static void destruct_data(DArena &);

        private:
            /** alloc driver. shared by alloc(), super_alloc(), sub_alloc() **/
            static value_type _alloc(DArena &,
                                     size_type z,
                                     alloc_mode mode);
        };

//        template <>
//        struct IAllocator_Impl<DArena> {
//            using ImplType = IAllocator_DArena;
//        };

    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_DArena.hpp */
