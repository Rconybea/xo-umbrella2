/** @file ObjectConverter.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "ObjectConverter.hpp"
#include "Integer.hpp"
#include "Float.hpp"
#include "Boolean.hpp"
#include "String.hpp"
#include "TaggedPtr.hpp"
#include "xo/alloc/Blob.hpp"

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeId;
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
            TaggedPtr
            object_to_int(IAlloc * mm, gp<Object> obj)
            {
                /* mm cannot be GC allocator!
                 * That's Object-only
                 */

                gp<Integer> int_obj = Integer::from(obj);

                if (!int_obj.get()) {
                    throw std::runtime_error(tostr("Object obj found where Integer expected",
                                                   xtag("obj", obj)));
                }

                /* allocate a Blob wrapper to satsify GC memory layout demands */

                gp<Blob> tmp = Blob::make(mm, sizeof(T));

                T * p = reinterpret_cast<T *>(tmp->data());

                *p = int_obj->value();

                /* WARNING: retval invalidated when *mm cleared/recycled/collected */

                return Reflect::make_tp(p);
            }

            template <typename T>
            gp<Object>
            float_to_object(IAlloc * mm, const TaggedPtr & src)
            {
                T * native = src.recover_native<T>();

                assert(native);

                return Float::make(mm, *native);
            }

            template <typename T>
            TaggedPtr
            object_to_float(IAlloc * mm, gp<Object> obj)
            {
                /* mm cannot be GC allocator!
                 * That's Object-only
                 */

                gp<Float> float_obj = Float::from(obj);

                if (!float_obj.get()) {
                    throw std::runtime_error(tostr("Object obj found where Float expected",
                                                   xtag("obj", obj)));
                }

                /* allocate a Blob wrapper to satsify GC memory layout demands */

                gp<Blob> tmp = Blob::make(mm, sizeof(T));

                T * p = reinterpret_cast<T *>(tmp->data());

                *p = float_obj->value();

                /* WARNING: retval invalidated when *mm cleared/recycled/collected */

                return Reflect::make_tp(p);
            }

            gp<Object>
            bool_to_object(IAlloc * /*mm*/, const TaggedPtr & src)
            {
                bool * native = src.recover_native<bool>();

                assert(native);

                return Boolean::boolean_obj(*native);
            }

            TaggedPtr
            object_to_bool(IAlloc * /*mm*/, gp<Object> obj)
            {
                static bool s_true = true;
                static bool s_false = false;

                gp<Boolean> bool_obj = Boolean::from(obj);

                if (!bool_obj.get()) {
                    throw std::runtime_error(tostr("Object obj found where Boolean expected",
                                                   xtag("obj", obj)));
                }

                return Reflect::make_tp(bool_obj->value() ? &s_true : &s_false);
            }

            gp<Object>
            string_to_object(IAlloc * mm, const TaggedPtr & src)
            {
                std::string * native = src.recover_native<std::string>();

                assert(native);

                return String::copy(mm, native->c_str());
            }

            TaggedPtr
            object_to_string(IAlloc * /*mm*/, gp<Object> obj)
            {
                gp<String> string_obj = String::from(obj);

                if (!string_obj.get()) {
                    throw std::runtime_error(tostr("Object obj founcd where String expected",
                                                   xtag("obj", obj)));
                }

                // still don't have good solver for this yet
                assert(false);
            }
        }

        ObjectConverter::ObjectConverter()
        {
            this->establish_conversion<std::int32_t>(&int_to_object<std::int32_t>,
                                                     &object_to_int<std::int32_t>);
            this->establish_conversion<std::int64_t>(&int_to_object<std::int64_t>,
                                                     &object_to_int<std::int64_t>);

            this->establish_conversion<double>(&float_to_object<double>,
                                               &object_to_float<double>);

            this->establish_conversion<bool>(&bool_to_object,
                                             &object_to_bool);

            this->establish_conversion<std::string>(&string_to_object,
                                                    &object_to_string);
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
                    throw std::runtime_error(tostr("no to-object-converter available for instance of type",
                                                   xtag("id", x_tp.td()->id()),
                                                   xtag("name", x_tp.td()->short_name())));
                }

                return nullptr;
            }
        }

        TaggedPtr
        ObjectConverter::tp_from_object(IAlloc * mm, gp<Object> & obj, TypeId target_id, bool throw_flag)
        {
            const Converter * cvt = cvt_.lookup(target_id);

            if (cvt) {
                return (cvt->cvt_from_object_)(mm, obj);
            } else {
                if (throw_flag) {
                    throw std::runtime_error(tostr("no from-object-converter available for instance of type",
                                                   xtag("id", target_id)));
                }

                return TaggedPtr::universal_null();
            }
        }
    }
}

/* end ObjectConverter.cpp */
