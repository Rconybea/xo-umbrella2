/* @file JsonPrinter.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/TypeDrivenMap.hpp"
#include "xo/reflect/TaggedPtr.hpp"
//#include <memory>
#include <iostream>

namespace xo {
  namespace json {
    class PrintJson;

    class JsonPrinter {
    public:
      using Reflect = xo::reflect::Reflect;
      using TaggedPtr = xo::reflect::TaggedPtr;
      using TypeDescr = xo::reflect::TypeDescr;
      using TypeId = xo::reflect::TypeId;

    public:
      JsonPrinter(PrintJson const * pjson) : pjson_{pjson} {}
      virtual ~JsonPrinter() = default;

      PrintJson const * pjson() const { return pjson_; }

      /* print tagged pointer in json format */
      virtual void print_json(TaggedPtr tp,
                  std::ostream * p_os) const = 0;

      void report_internal_type_consistency_error(TypeDescr td1,
                          TypeDescr td2,
                          std::ostream * p_os) const;

      /* convenience method for derived printers.
       * retrieves contents of tp as a T*,  complains to *p_os if that fails.
       *
       * (Failure would occur if printer for type T was instead installed
       *  for some unrelated type U)
       */
      template<typename T>
      T * check_recover_native(TaggedPtr tp, std::ostream * p_os) const {
    T * x = tp.recover_native<T>();

    if (!x) {
      this->report_internal_type_consistency_error(Reflect::require<T>(),
                               tp.td(),
                               p_os);
    }

    return x;
      } /*check_recover_native*/

      void assign_pjson(PrintJson const * pjson) {
    assert(this->pjson_ == nullptr || this->pjson_ == pjson);
    this->pjson_ = pjson;
      } /*assign_pjson*/

    private:
      /* a json printers is installed into one PrintJson instance;
       * capture address of that instance at install time
       */
      PrintJson const * pjson_ = nullptr;
    }; /*JsonPrinter*/

    /* AsStringJsonPrinter<T>
     * prints a T-instance by using operator<< and surrounding in quotes.
     *
     * e.g:
     *   T & x = ..;
     *   std::ostream * p_os = ..;
     *
     *   *p_os << "\"" << x << "\""
     *
     */
    template<typename T>
    class AsStringJsonPrinter : public JsonPrinter {
    public:
      AsStringJsonPrinter(PrintJson const * pjson) : JsonPrinter(pjson) {}

      virtual void print_json(TaggedPtr tp,
                  std::ostream * p_os) const override
      {
    T * x = this->check_recover_native<T>(tp, p_os);

    if(x) {
      *p_os << "\"" << *x << "\"";
    }
      } /*print_json*/
    }; /*AsStringJsonPrinter*/
  } /*namespace json*/
} /*namespace xo*/


/* end JsonPrinter.hpp */
