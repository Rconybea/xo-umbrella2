/** @file TypeOps.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "AtomicType.hpp"
#include "ListType.hpp"
#include "ArrayType.hpp"
#include <xo/alloc2/Allocator.hpp>

namespace xo {
    namespace scm {
        class TypeOps {
        public:
            using AAllocator = xo::mm::AAllocator;

            template <typename AFacet = AType>
            static obj<AFacet,DAtomicType> atomic_type(obj<AAllocator> mm, Metatype metatype) {
                assert(metatype.is_atomic());
                return obj<AFacet,DAtomicType>(DAtomicType::_make(mm, metatype));
            }

            template <typename AFacet = AType>
            static obj<AFacet,DListType> list_type(obj<AAllocator> mm, obj<AType> elt_type) {
                return obj<AFacet,DListType>(DListType::_make(mm, elt_type));
            }

            template <typename AFacet = AType>
            static obj<AFacet,DArrayType> array_type(obj<AAllocator> mm, obj<AType> elt_type) {
                return obj<AFacet,DArrayType>(DArrayType::_make(mm, elt_type));
            }
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end TypeOps.hpp */
