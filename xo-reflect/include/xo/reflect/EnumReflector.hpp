/** @file EnumReflector.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "Reflect.hpp"
#include "TypeDescr.hpp"
#include "enum/EnumTdx.hpp"
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace xo {
    namespace reflect {
        /** @brief describe enum @p EnumT to xo-reflect: its enumerators, by
         *  name and value.
         *
         *  Like StructReflector, for an enum.  Use:
         *
         *    enum class Runstate { stopped, stop_requested, running };
         *
         *    EnumReflector<Runstate> er;
         *
         *    if (er.is_incomplete()) {
         *        REFLECT_ENUM(er, stopped);
         *        REFLECT_ENUM(er, stop_requested);
         *        REFLECT_ENUM(er, running);
         *    }
         *    // completes when er goes out of scope (or: er.require_complete())
         *
         *  Then Reflect::require<Runstate>()->is_enum() is true, and its
         *  enum_info() converts between values and names.  Its metatype stays
         *  mt_atomic: an enum composes no other type.
         *
         *  An enum has no member functions to put this in, so a companion
         *  does it (e.g. RunstateUtil::reflect_self).
         *
         *  See .xo-backlog/xo-reflect/issues/06.
         **/
        template <typename EnumT>
        class EnumReflector {
        public:
            static_assert(std::is_enum_v<EnumT>, "EnumReflector: EnumT must be an enum");
            static_assert(sizeof(std::underlying_type_t<EnumT>) <= sizeof(std::int64_t),
                          "EnumReflector: values travel as std::int64_t");

            using enum_t = EnumT;
            using underlying_t = std::underlying_type_t<EnumT>;

        public:
            EnumReflector() : td_{EstablishTypeDescr::establish<EnumT>()} {}
            ~EnumReflector() {
                this->require_complete();
            }

            bool is_complete() const { return s_reflected_flag; }
            bool is_incomplete() const { return !s_reflected_flag; }
            TypeDescr td() const { return td_; }

            /** enumerator @p value, named @p name; in declaration order **/
            void reflect_enumerator(std::string const & name, EnumT value) {
                this->enum_v_.push_back(EnumTdx::Enumerator{to_int64(value), name});
            }

            /** install the enumerators collected so far; once per type **/
            void require_complete() {
                if (!s_reflected_flag) {
                    s_reflected_flag = true;

                    static detail::InvokerAux<EnumT> s_final_invoker;

                    this->td_->assign_tdextra(&s_final_invoker,
                                              EnumTdx::make(std::move(this->enum_v_),
                                                            &load, &store));
                }
            } /*require_complete*/

        private:
            static std::int64_t to_int64(EnumT x) {
                return static_cast<std::int64_t>(static_cast<underlying_t>(x));
            }

            /** EnumTdx::LoadFn for EnumT **/
            static std::int64_t load(void const * object) {
                return to_int64(*static_cast<EnumT const *>(object));
            }

            /** EnumTdx::StoreFn for EnumT **/
            static void store(void * object, std::int64_t value) {
                *static_cast<EnumT *>(object) = static_cast<EnumT>(static_cast<underlying_t>(value));
            }

        private:
            /* set once EnumT's EnumTdx is installed */
            static bool s_reflected_flag;

            /* type description for EnumT */
            TypeDescrW td_;
            /* enumerators, in declaration order */
            std::vector<EnumTdx::Enumerator> enum_v_;
        }; /*EnumReflector*/

        template <typename EnumT>
        bool EnumReflector<EnumT>::s_reflected_flag = false;
    } /*namespace reflect*/

    /* e.g.
     *   enum class Runstate { stopped, .. };
     *
     *   EnumReflector<Runstate> er;
     *   REFLECT_EXPLICIT_ENUM(er, "Stopped", Runstate::stopped);
     */
#define REFLECT_EXPLICIT_ENUM(er, enumerator_name, enumerator) er.reflect_enumerator(enumerator_name, enumerator)

    /* e.g.
     *   enum class Runstate { stopped, .. };
     *
     *   EnumReflector<Runstate> er;
     *   REFLECT_ENUM(er, stopped);
     *
     * expands to something like:
     *   er.reflect_enumerator("stopped", EnumReflector<Runstate>::enum_t::stopped)
     *
     * for a scoped enum or an unscoped one
     */
#define REFLECT_ENUM(er, enumerator) er.reflect_enumerator(#enumerator, decltype(er)::enum_t::enumerator)

} /*namespace xo*/

/* end EnumReflector.hpp */
