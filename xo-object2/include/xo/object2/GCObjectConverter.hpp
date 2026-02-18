/** @file GCObjectConverter.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include <xo/gc/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/reflect/Reflect.hpp>
//#include "xo/reflect/TaggedPtr.hpp"
#include <xo/reflect/TypeDrivenMap.hpp>

namespace xo {
    namespace scm {
        /* Convert between xo::reflect::TaggedPtr and xo::Object for
         * a particular wrapped c++ type
         */
        struct Converter {
            using AAllocator = xo::mm::AAllocator;
            using AGCObject = xo::mm::AGCObject;
            using TaggedPtr = xo::reflect::TaggedPtr;
            /** convert from some reflected T* @p src to
             *  obj<AGCObject> dest
             *  using memory from allocator @p mm
             **/
            using ConvertToObjectFn = obj<AGCObject> (*)(obj<AAllocator> mm,
                                                         TaggedPtr src);
            /** convert from obj<AGCObject> @p src to some refected T* @p dest
             *  using memory from allocator @p mm.
             *
             *  NOTE: obj<AGCObject> is gc-aware -> will likely reside in
             *        a collected memory region.
             **/
            using ConvertFromObjectFn = TaggedPtr (*)(obj<AAllocator> mm,
                                                      obj<AGCObject> obj);

        public:
            Converter() = default;
            explicit Converter(ConvertToObjectFn to,
                               ConvertFromObjectFn from)
                : cvt_to_object_{to},
                  cvt_from_object_{from}
                {}

            /** convert tagged pointer @p tp to new object,
             *  allocated via @p mm.
             *
             *  Conversion will typically be for some specific type;
             *  see @ref ObjectConverter
             **/
            ConvertToObjectFn cvt_to_object_ = nullptr;

            /** convert object to tagged pointer @p,
             *  allocated via @p mm.
             *
             *  Conversion will typically be for some specific type;
             *  see @ref ObjectConverter
             **/
            ConvertFromObjectFn cvt_from_object_ = nullptr;
        };

        /** @class ObjectConverter
         *  @brief Conversion to/from Object
         *
         *  For some instance of type T:
         *
         *  @code
         *  ObjectConverter & converters = ...;
         *  T x = ...;
         *  TaggedPtr tp = Reflect::make_tp(&x);
         *  TypeId tid = tp.td()->id();
         *
         *  const Converter * cvt = converters.cvt_.lookup(tid);
         *
         *  if (cvt) {
         *    // cvt is a converter for T instances
         *    gp<Object> obj = (*(cvt->cvt_to_object_))(mm,
         *  @endcode
         **/
        class ObjectConverter {
        public:
            using AAllocator = xo::mm::AAllocator;
            using AGCObject = xo::mm::AGCObject;
            using Reflect = xo::reflect::Reflect;
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TypeId = xo::reflect::TypeId;

            /** sets up standard conversions **/
            ObjectConverter();

            /** singleton instance **/
            static const ObjectConverter & instance();

            /** establish conversion: use @p fn to convert values of type @tparam T. **/
            template <typename T>
            void establish_conversion(Converter::ConvertToObjectFn to,
                                      Converter::ConvertFromObjectFn from);

            /** convert tagged poitner @p tp to object.  allocates memory only from @p mm.
             *  return nullptr if no converter available and @p throw_flag not set.
             *  throw exception if no converter available and @p throw_flag set.
             **/
            obj<AGCObject> tp_to_gco(obj<AAllocator> mm,
                                     TaggedPtr tp,
                                     bool throw_flag) const;

            /** convert @p x to object.
             *  return converted object; if allocated, using only memory from @p mm.
             *  return nullptr if no converter available, and @p throw_flag not set.
             *  throw exception if no converter available, and @p throw_flag set.
             **/
            template <typename T>
            obj<AGCObject> to_gco(obj<AAllocator> mm, const T & x, bool throw_flag);

            /** convert object @p obj to tagged pointer, with typeid @p target_id.
             *  Allocates memory only from @p mm.
             *  return null TaggedPtr if no converter available and @p throw_flag not set.
             *  Throw exception if no converter available and @p throw_flag set.
             **/
            TaggedPtr tp_from_gco(obj<AAllocator> mm,
                                  obj<AGCObject> obj,
                                  TypeId target_type_id,
                                  bool throw_flag) const;

        private:
            /** expandable type-driven conversion table.
             **/
            xo::reflect::TypeDrivenMap<Converter> cvt_;
        };

        template <typename T>
        void
        ObjectConverter::establish_conversion(Converter::ConvertToObjectFn to,
                                              Converter::ConvertFromObjectFn from)
        {
            using xo::reflect::TypeDescrW;
            using xo::reflect::Reflect;

            TypeDescrW td = Reflect::require<T>();
            Converter * cvt = cvt_.require(td);

            *cvt = Converter(to, from);
        }

        template <typename T>
        auto
        ObjectConverter::to_gco(obj<AAllocator> mm, const T & x, bool throw_flag)
            -> obj<AGCObject>
        {
            TaggedPtr x_tp = Reflect::make_tp(&x);

            return tp_to_gco(mm, x_tp, throw_flag);
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end GCObjectConverter.hpp */
