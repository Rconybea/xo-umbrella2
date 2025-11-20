/** @file ObjectConverter.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "ObjectConverter.hpp"
#include "Integer.hpp"

namespace xo {
    using xo::reflect::TaggedPtr;
    using xo::gc::IAlloc;

    namespace obj {
        namespace {
            template <typename T>
            gp<Object>
            int_to_object(IAlloc * mm, const TaggedPtr & src)
            {
                T * native = src.recover_native<T>();

                assert(native);

                return Integer::make(mm, *native);
            }
        }

        ObjectConverter::ObjectConverter()
        {
            this->establish_conversion<std::int32_t>(&int_to_object<std::int32_t>);
        }
    }
}

/* end ObjectConverter.cpp */
