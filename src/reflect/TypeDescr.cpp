/* @file TypeDescr.cpp */

#include "TypeDescr.hpp"
#include "TaggedPtr.hpp"
#include "TypeDescrExtra.hpp"
#include "Reflect.hpp"
#include "atomic/AtomicTdx.hpp"
#include "function/FunctionTdx.hpp"
#include "xo/indentlog/scope.hpp"

namespace xo {
    using xo::scope;
    using xo::xtag;
    using xo::tostr;

    namespace reflect {
        uint32_t
        TypeId::s_next_id = 1;

        std::string
        FunctionTdxInfo::make_canonical_name() const
        {
            std::ostringstream ss;

            ss << retval_td_->canonical_name();
            ss << " (*)(";
            for (std::size_t i = 0, n = arg_td_v_.size(); i < n; ++i) {
                if (i > 0)
                    ss << ",";
                ss << arg_td_v_[i]->canonical_name();
            }
            ss << ")";

            return ss.str();
        } /*make_canonical_name*/

        // ----- TypeDescrBase -----

        std::unordered_map<FunctionTdxInfo, TypeDescrBase*>
        TypeDescrBase::s_function_type_map;

        std::unordered_map<std::string, TypeDescrBase*>
        TypeDescrBase::s_canonical_type_table_map;

        std::unordered_map<TypeInfoRef, TypeDescrBase*>
        TypeDescrBase::s_native_type_table_map;

        std::unordered_map<TypeInfoRef, TypeDescrBase*>
        TypeDescrBase::s_coalesced_type_table_map;

        std::vector<std::unique_ptr<TypeDescrBase>>
        TypeDescrBase::s_type_table_v;

        TypeDescrW
        TypeDescrBase::require(const std::type_info * native_tinfo,
                               const std::string & canonical_name,
                               detail::Invoker * invoker,
                               std::unique_ptr<TypeDescrExtra> tdextra)
        {
            scope log(XO_DEBUG(false));

            log && log(xtag("canonical_name", canonical_name));

            if (native_tinfo) {
                /* 1. lookup by tinfo hash_code in s_type_table_map
                 *    Not available for manually-constructed type descriptions.
                 */
                {
                    auto ix = s_native_type_table_map.find(TypeInfoRef(native_tinfo));

                    if ((ix != s_native_type_table_map.end()) && ix->second) {
                        log && log("TypeDescrBase::require"
                                   ": using s_native_type_table_map[TypeInfoRef(native_tinfo)]");

                        return ix->second;
                    }
                }

                /* 2. lookup by tinfo hash_code in s_coalesced_type_table_map */
                {
                    auto ix = s_coalesced_type_table_map.find(TypeInfoRef(native_tinfo));

                    if ((ix != s_coalesced_type_table_map.end()) && ix->second) {
                        log && log("TypeDescrBase::require"
                                   ": using s_coalesced_type_table_map[TypeInfoRef(native_tinfo)]");

                        return ix->second;
                    }
                }
            }

            log && log("TypeDescrBase::require: try lookup by canonical name");

            /* 3. lookup by canonical_name,  before we create a new slot.
             *
             * Have to accept that on clang type_info objects aren't always unique (!$@#!!)
             */
            {
                auto ix = s_canonical_type_table_map.find(canonical_name);

                if (ix != s_canonical_type_table_map.end()) {

                    /** assume existing slot, with same canonical name,
                     *  represents the same type as native_tinfo
                     **/
                    if (native_tinfo) {
                        auto existing_tinfo = ix->second->native_typeinfo();

                        /* given we have a match:
                         * - on existing TypeDescr
                         * - with same canonical name as type assoc'd with native_tinfo
                         * then:
                         *   it's possible existing TypeDescr was manually constructed
                         *   (i.e. without capturing std::type_info).
                         *
                         * With that in mind, attach that typeinfo now
                         */
                        if (!existing_tinfo) {
                            ix->second->assign_native_tinfo(native_tinfo);

                            s_native_type_table_map[TypeInfoRef(native_tinfo)]
                                = ix->second;
                        }

                        if (existing_tinfo
                            && (existing_tinfo != native_tinfo))
                        {
                            /* we have encountered distinct std::type_info objects
                             * that appear to represent the same type.
                             * (at least types with the same canonical name)
                             *
                             * We observe this happening sometimes with clang-prepared
                             * shared libraries;  perhaps something going wrong with
                             * symbol coalescing.
                             *
                             * Store the dups in s_coalesced_type_table_map for future reference.
                             */
                            auto jx = s_coalesced_type_table_map.find(TypeInfoRef(native_tinfo));

                            if (jx == s_coalesced_type_table_map.end())
                                s_coalesced_type_table_map[TypeInfoRef(native_tinfo)]
                                    = ix->second;
                        }
                    }

                    return ix->second;
                }
            }

            /* when control here:
             * need type added to:
             * - s_type_table_v
             * - s_canonical_type_table_map
             * - s_native_type_table_map
             * - s_coalesced_type_table_map (omit, only used for dups)
             * - s_function_type_map (if type represents a function)
             */

            /* allocate slot for a new TypeDescr instance: */

            TypeId new_td_id = TypeId::allocate();

            log && log("TypeDescrBase::require", xtag("new_td_id", new_td_id));

            if (s_type_table_v.size() <= new_td_id.id())
                s_type_table_v.resize(new_td_id.id() + 1);

            auto & new_slot = s_type_table_v[new_td_id.id()];

            auto new_td = new TypeDescrBase(new_td_id,
                                            native_tinfo,
                                            canonical_name,
                                            std::move(tdextra),
                                            invoker);

            new_slot.reset(new_td);

            s_canonical_type_table_map[std::string(new_slot->canonical_name())] = new_td;
            if (native_tinfo)
                s_native_type_table_map[TypeInfoRef(native_tinfo)] = new_td;

            if (new_td->tdextra() && new_td->is_function()) {
                s_function_type_map[*(new_td->fn_info())] = new_td;
            }

            return new_slot.get();
        } /*require*/

        TypeDescrW
        TypeDescrBase::require_by_fn_info(const FunctionTdxInfo & fn_info) {
            auto ix = s_function_type_map.find(fn_info);

            if (ix != s_function_type_map.end())
                return ix->second;

            auto fn_tdextra = FunctionTdx::make_function(fn_info);

            return require(nullptr /*native_tinfo - n/avail on this path*/,
                           fn_info.make_canonical_name(),
                           nullptr /*invoker*/,
                           std::move(fn_tdextra));
        } /*require_by_fn_info*/

        TypeDescr
        TypeDescrBase::lookup_by_name(const std::string & name) {
            auto ix = s_canonical_type_table_map.find(name);

            if (ix == s_canonical_type_table_map.end()) {
                throw std::runtime_error(tostr("TypeDescrBase::lookup_by_name"
                                               ": no registered type with canonical name T",
                                               xtag("T", name)));
            }

            return ix->second;
        } /*lookup_by_name*/

        void
        TypeDescrBase::print_reflected_types(std::ostream & os)
        {
            os << "<type_table_v[" << s_type_table_v.size() << "]:";

            for (const auto & td : s_type_table_v) {
                os << "\n ";
                if (td) {
                    td->display(os);
                }
            }

            os << ">\n";
        } /*print_reflected_types*/

        namespace {
            /* readability hack:
             *    foo::bar::Quux ==> Quux
             * but lookout for template names:
             *    std::pair<std::bar, std::foo> ==> pair<std::bar, std::foo>
             */
            std::string_view
            unqualified_name(std::string_view const & canonical_name)
            {
                size_t m = canonical_name.find_first_of('<');

                /* skip ':', but only in range [0..m) */
                size_t p = canonical_name.find_last_of(':', m);

                if (p == std::string_view::npos) {
                    return canonical_name;
                } else {
                    if ((canonical_name.substr(0, 9) == "std::pair")
                        || (canonical_name.substr(0, 13) == "std::_1::pair"))
                    {
                        return std::string_view("pair");
                    } else {
                        return std::string_view(canonical_name.substr(p+1));
                    }
                }
            } /*unqualified_name*/
        } /*namespace*/

        TypeDescrBase::TypeDescrBase(TypeId id,
                                     const std::type_info * native_tinfo,
                                     const std::string & canonical_name,
                                     std::unique_ptr<TypeDescrExtra> tdextra,
                                     detail::Invoker * invoker)
            : id_{std::move(id)},
              native_typeinfo_{native_tinfo},
              canonical_name_{std::move(canonical_name)},
              short_name_{unqualified_name(canonical_name_)},
              invoker_{invoker},
              tdextra_{std::move(tdextra)}
        {
        }

        bool
        TypeDescrBase::is_i64() const
        {
            // FIXME: on apple/clang may need to use long long

            static_assert(sizeof(std::int64_t) == 8);

            return Reflect::is_native<std::int64_t>(this);
        }

        bool
        TypeDescrBase::is_f64() const
        {
            static_assert(sizeof(double) == 8);

            return Reflect::is_native<double>(this);
        }

        TaggedPtr
        TypeDescrBase::most_derived_self_tp(void * object) const
        {
            return this->tdextra_->most_derived_self_tp(this, object);
        } /*most_derived_self_tp*/

        TaggedPtr
        TypeDescrBase::child_tp(uint32_t i, void * object) const
        {
            return this->tdextra_->child_tp(i, object);
        } /*child_tp*/

        void
        TypeDescrBase::display(std::ostream & os) const
        {
            os << "<TypeDescr"
               << xtag("id", id_)
               << xtag("canonical_name", canonical_name_)
               << xtag("complete", complete_flag_)
               << xtag("metatype", this->metatype())
               << ">";
        } /*display*/

        std::string
        TypeDescrBase::display_string() const
        {
            return tostr(*this);
        } /*display_string*/

        bool
        TypeDescrBase::mark_complete()
        {
            bool retval = this->complete_flag_;

            this->complete_flag_ = true;

            return retval;
        } /*mark_complete*/

#ifdef NOT_USING
        void
        TypeDescrBase::assign_tdextra(std::unique_ptr<TypeDescrExtra> tdx)
        {
            scope log(XO_ENTER0(verbose),
                      xtag("canonical_name", this->canonical_name()),
                      xtag("tdextra.old", this->tdextra_.get()),
                      xtag("metatype.old", (this->tdextra_
                                            ? this->tdextra_->metatype()
                                            : Metatype::mt_invalid)),
                      xtag("metatype.new", tdx->metatype()));

            this->complete_flag_ = true;
            this->tdextra_ = std::move(tdx);
        } /*assign_tdextra*/
#endif

        TypeDescrTable::TypeDescrTable() {
            /* want long == i64 */
            static_assert(sizeof(long) == 8);
            /* want double == f64 */
            static_assert(sizeof(double) == 8);

            Reflect::require<bool>();
            Reflect::require<char>();
            Reflect::require<short>();
            Reflect::require<int>();
            Reflect::require<long>();
            Reflect::require<long long>();
            Reflect::require<float>();
            Reflect::require<double>();
            Reflect::require<std::string>();
        } /*ctor*/

        TypeDescrTable
        TypeDescrTable::s_instance;
    } /*namespace reflect*/
} /*namespace xo*/

/* end TypeDescr.cpp */
