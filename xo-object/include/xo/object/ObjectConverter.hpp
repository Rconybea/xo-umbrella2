/** @file ObjectConverter.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "Object.hpp"
#include "xo/reflect/Reflect.hpp"
//#include "xo/reflect/TaggedPtr.hpp"
#include "xo/reflect/TypeDrivenMap.hpp"

namespace xo {
    namespace obj {
        /* Convert between xo::reflect::TaggedPtr and xo::Object for
         * a particular wrapped c++ type.
         */
        struct Converter {
            using TaggedPtr = xo::reflect::TaggedPtr;
            using ConvertToObjectFn = gp<Object> (*)(gc::IAlloc *, const TaggedPtr &);
            using ConvertFromObjectFn = TaggedPtr (*)(gc::IAlloc *, gp<Object> obj);

        public:
            Converter() = default;
            explicit Converter(ConvertToObjectFn to,
                               ConvertFromObjectFn from)
                : cvt_to_object_{to}, cvt_from_object_{from} {}

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
         *
         *  ObjectConverter converts at run-time
         *  @see ObjectConversion for compile-time conversion
         **/
        class ObjectConverter {
        public:
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TypeId = xo::reflect::TypeId;
            using IAlloc = xo::gc::IAlloc;

            /** sets up standard conversions **/
            ObjectConverter();

            /** establish conversion: use @p fn to convert values of type @tparam T. **/
            template <typename T>
            void establish_conversion(Converter::ConvertToObjectFn to, Converter::ConvertFromObjectFn from);

            /** convert tagged poitner @p tp to object.  allocates memory only from @p mm.
             *  return nullptr if no converter available and @p throw_flag not set.
             *  throw exception if no converter available and @p throw_flag set.
             **/
            gp<Object> tp_to_object(IAlloc * mm, const TaggedPtr & tp, bool throw_flag);

            /** convert @p x to object.
             *  return converted object; if allocated, using only memory from @p mm.
             *  return nullptr if no converter available, and @p throw_flag not set.
             *  throw exception if no converter available, and @p throw_flag set.
             **/
            template <typename T>
            gp<Object> to_object(IAlloc * mm, const T & x, bool throw_flag);

            /** convert object @p obj to tagged pointer, with typeid @target_id.
             *  Allocates memory only from @p mm.
             *  return null TaggedPtr if no converter available and @p throw_flag not set.
             *  Throw exception if no converter available and @p throw_flag set.
             **/
            TaggedPtr tp_from_object(IAlloc * mm, gp<Object> & obj, TypeId target_id, bool throw_flag);

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
        gp<xo::Object>
        ObjectConverter::to_object(IAlloc * mm, const T & x, bool throw_flag)
        {
            using xo::reflect::Reflect;
            using xo::reflect::TaggedPtr;

            TaggedPtr x_tp = Reflect::make_tp(&x);

            return tp_to_object(mm, x_tp, throw_flag);
        }
    }
}

/* end ObjectConverter.hpp */
