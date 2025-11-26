/** @file ObjectConverter.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "ObjectConverter.hpp"
#include "Integer.hpp"
#include "Float.hpp"
#include "Boolean.hpp"

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

            template <typename T>
            gp<Object>
            float_to_object(IAlloc * mm, const TaggedPtr & src)
            {
                T * native = src.recover_native<T>();

                assert(native);

                return Float::make(mm, *native);
            }

            gp<Object>
            bool_to_object(IAlloc * /*mm*/, const TaggedPtr & src)
            {
                bool * native = src.recover_native<bool>();

                assert(native);

                return Boolean::boolean_obj(*native);
            }
        }

        ObjectConverter::ObjectConverter()
        {
            this->establish_conversion<std::int32_t>(&int_to_object<std::int32_t>);
            this->establish_conversion<std::int64_t>(&int_to_object<std::int64_t>);

            this->establish_conversion<double>(&float_to_object<double>);

            this->establish_conversion<bool>(&bool_to_object);
        }

        gp<Object>
        ObjectConverter::tp_to_object(IAlloc * mm, const TaggedPtr & x_tp, bool throw_flag)
        {
            using xo::reflect::Reflect;
            using xo::reflect::TaggedPtr;

            const Converter * cvt = cvt_.lookup(x_tp.td());

            if (cvt) {
                return (cvt->cvt_to_object_)(mm, x_tp);
            } else {
                if (throw_flag) {
                    throw std::runtime_error(tostr("no object-converter available for instance of type",
                                                   xtag("id", x_tp.td()->id()),
                                                   xtag("name", x_tp.td()->short_name())));
                }

                return nullptr;
            }
        }
    }
}

/* end ObjectConverter.cpp */
