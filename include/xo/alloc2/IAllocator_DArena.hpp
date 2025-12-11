/** @file IAllocator_DArena.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AAllocator.hpp"
#include "DArena.hpp"

namespace xo {
    namespace mm {

        template <>
        struct IAllocator_Impl<DArena> {
            static const std::string & name(const DArena & s) {
                return s.name_;
            }

            static std::size_t size(const DArena & s);
            static std::size_t committed(const DArena & s);
            static bool contains(const DArena & s, const void * p);
            static std::byte * alloc(const DArena & s, std::size_t z);
            static void clear(DArena & s);
            static void destruct_data(DArena & s);
        };


    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_DArena.hpp */
