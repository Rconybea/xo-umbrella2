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
        struct Converter {
            using TaggedPtr = xo::reflect::TaggedPtr;
            using ConvertToObjectFn = gp<Object> (*)(gc::IAlloc *, const TaggedPtr &);

        public:
            explicit Converter(ConvertToObjectFn f) : cvt_to_object_{f} {}

            /** convert tagged pointer @p tp to new object,
             *  allocated via @p mm.
             *
             *  Conversion will typically be for some specific type;
             *  see @ref ObjectConverter
             **/
            ConvertToObjectFn cvt_to_object_;
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
            using IAlloc = xo::gc::IAlloc;

            /** sets up standard conversions **/
            ObjectConverter();

            /** establish conversion: use @p fn to convert values of type @tparam T. **/
            template <typename T>
            void establish_conversion(Converter::ConvertToObjectFn fn);

            /** convert @p x to object.
             *  return converted object; if allocated, using only memory from @p mm.
             *  return nullptr if no converter available, and @p throw_flag not set.
             *  throw exception if no converter available, and @p throw_flag set.
             **/
            template <typename T>
            gp<Object> to_object(IAlloc * mm, const T & x, bool throw_flag);

        private:
            /** expandable type-driven conversion table.
             **/
            xo::reflect::TypeDrivenMap<Converter> cvt_;
        };

        template <typename T>
        void
        ObjectConverter::establish_conversion(Converter::ConvertToObjectFn fn)
        {
            using xo::reflect::TypeDescrW;
            using xo::reflect::Reflect;

            TypeDescrW td = Reflect::require<T>();
            Converter * cvt = cvt_.require(td);

            *cvt = Converter(fn);
        }

        template <typename T>
        gp<xo::Object>
        ObjectConverter::to_object(IAlloc * mm, const T & x, bool throw_flag)
        {
            using xo::reflect::Reflect;
            using xo::reflect::TaggedPtr;

            TaggedPtr x_tp = Reflect::make_tp(&x);
            Converter * cvt = cvt_.lookup(x_tp.td());

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

/* end ObjectConverter.hpp */
