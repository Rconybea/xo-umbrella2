/** @file GCObjectConverter.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "GCObjectConverter.hpp"

#include "DInteger.hpp"
#include "number/IGCObject_DInteger.hpp"

#include "DFloat.hpp"
#include "number/IGCObject_DFloat.hpp"

#include "DBoolean.hpp"
#include "boolean/IGCObject_DBoolean.hpp"

#include "DString.hpp"
#include "string/IGCObject_DString.hpp"

#include <xo/facet/obj.hpp>
#include <xo/reflect/TaggedPtr.hpp>
//#include "xo/alloc/Blob.hpp"

namespace xo {
    using xo::mm::AGCObject;
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeId;
    using xo::facet::obj;
    using xo::facet::typeseq;
    using xo::mm::AAllocator;

    namespace scm {
        namespace {
            // DInteger <-> T

            template <typename T>
            obj<AGCObject>
            int_to_gco(obj<AAllocator> mm, TaggedPtr src)
            {
                T * native = src.recover_native<T>();

                assert(native);

                return DInteger::box<AGCObject>(mm, *native);
            }

            template <typename T>
            TaggedPtr
            gco_to_int(obj<AAllocator> mm,
                       obj<AGCObject> obj)
            {

                /* mm cannot be GC allocator!
                 * That's Object-only
                 */

                auto int_obj = xo::facet::obj<AGCObject,DInteger>::from(obj); //Integer::from(obj);

                if (!int_obj) {
                    throw std::runtime_error
                        (tostr("Object obj found where Integer expected",
                               xtag("obj", obj)));
                }

                void * mem = mm.alloc(typeseq::id<T>(), sizeof(T));

                T * p = reinterpret_cast<T *>(mem);

                *p = int_obj.data()->value();

                /* Note:
                 *  retval invalidated when
                 *  *mm cleared/recycled/collected
                 */

                return Reflect::make_tp(p);
            }

            // DFloat <-> T

            template <typename T>
            xo::facet::obj<xo::mm::AGCObject>
            float_to_gco(xo::facet::obj<AAllocator> mm,
                         TaggedPtr src)
            {
                T * native = src.recover_native<T>();

                assert(native);

                return DFloat::box<AGCObject>(mm, *native);
            }

            template <typename T>
            TaggedPtr
            gco_to_float(obj<AAllocator> mm, obj<AGCObject> obj)
            {
                auto float_obj = xo::facet::obj<AGCObject,DFloat>::from(obj);

                if (!float_obj) {
                    throw std::runtime_error
                        (tostr("Object obj found where Float expected",
                               xtag("obj", obj)));
                }

                void * mem = mm.alloc(typeseq::id<T>(), sizeof(T));

                T * p = reinterpret_cast<T *>(mem);

                *p = float_obj.data()->value();

                /* Note:
                 *   retval invalidated when *mm cleared/recycled/collected
                 */

                return Reflect::make_tp(p);
            }

            // DBoolean <-> T

            obj<AGCObject>
            bool_to_gco(obj<AAllocator> mm, TaggedPtr src)
            {
                bool * native = src.recover_native<bool>();

                assert(native);

                return DBoolean::box<AGCObject>(mm, *native);
            }

            TaggedPtr
            gco_to_bool(obj<AAllocator> /*mm*/,
                        obj<AGCObject> obj)
            {
                static bool s_true = true;
                static bool s_false = false;

                auto bool_obj = xo::facet::obj<AGCObject,DBoolean>::from(obj);

                if (!bool_obj) {
                    throw std::runtime_error
                        (tostr("Object obj found where Boolean expected",
                               xtag("obj", obj)));
                }

                return Reflect::make_tp(bool_obj.data()->value() ? &s_true : &s_false);
            }

            // DString <-> T
            // w/
            //   T = std::string

            obj<AGCObject>
            string_to_gco(obj<AAllocator> mm, TaggedPtr src)
            {
                // try std::string..

                std::string * native = src.recover_native<std::string>();

                assert(native);

                DString * dstr = DString::from_str(mm, *native);

                return xo::facet::obj<AGCObject,DString>(dstr);
            }

            TaggedPtr
            gco_to_string(obj<AAllocator> mm, obj<AGCObject> obj)
            {
                auto string_obj = xo::facet::obj<AGCObject,DString>::from(obj);

                if (!string_obj) {
                    throw std::runtime_error
                        (tostr("Object obj founcd where String expected",
                               xtag("obj", obj)));
                }

                // still don't have impl for this
                // Need regular std::allocator interface
                //

                (void)mm;
                assert(false);
            }
        }

        ObjectConverter::ObjectConverter()
        {
            this->establish_conversion<std::int32_t>(&int_to_gco<std::int32_t>,
                                                     &gco_to_int<std::int32_t>);
            this->establish_conversion<std::int64_t>(&int_to_gco<std::int64_t>,
                                                     &gco_to_int<std::int64_t>);

            this->establish_conversion<double>(&float_to_gco<double>,
                                               &gco_to_float<double>);

            this->establish_conversion<bool>(&bool_to_gco,
                                             &gco_to_bool);

            this->establish_conversion<std::string>(&string_to_gco,
                                                    &gco_to_string);
        }

        const ObjectConverter &
        ObjectConverter::instance() {
            static ObjectConverter s_instance;

            return s_instance;
        }

        obj<AGCObject>
        ObjectConverter::tp_to_gco(obj<AAllocator> mm,
                                   TaggedPtr x_tp,
                                   bool throw_flag) const
        {
            using xo::reflect::Reflect;
            using xo::reflect::TaggedPtr;

            const Converter * cvt = cvt_.lookup(x_tp.td());

            if (cvt) {
                return (cvt->cvt_to_object_)(mm, x_tp);
            } else {
                if (throw_flag) {
                    throw std::runtime_error
                        (tostr("no to-object-converter available for instance of type",
                               xtag("id", x_tp.td()->id()),
                               xtag("name", x_tp.td()->short_name())));
                }

                return obj<AGCObject>();
            }
        }

        TaggedPtr
        ObjectConverter::tp_from_gco(obj<AAllocator> mm,
                                     obj<AGCObject> obj,
                                     TypeId target_id,
                                     bool throw_flag) const
        {
            const Converter * cvt = cvt_.lookup(target_id);

            if (cvt) {
                return (cvt->cvt_from_object_)(mm, obj);
            } else {
                if (throw_flag) {
                    throw std::runtime_error
                        (tostr("no from-object-converter available for instance of type",
                               xtag("id", target_id)));
                }

                return TaggedPtr::universal_null();
            }
        }
    }
}

/* end GCObjectConverter.cpp */
