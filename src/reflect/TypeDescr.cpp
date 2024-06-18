/* @file TypeDescr.cpp */

#include "TypeDescr.hpp"
#include "TaggedPtr.hpp"
#include "TypeDescrExtra.hpp"
#include "atomic/AtomicTdx.hpp"
#include "xo/indentlog/scope.hpp"

namespace xo {
    using xo::scope;
    using xo::xtag;
    using xo::tostr;

    namespace reflect {
        uint32_t
        TypeId::s_next_id = 1;

        std::unordered_map<TypeInfoRef, std::unique_ptr<TypeDescrBase>>
        TypeDescrBase::s_type_table_map;

        std::unordered_map<TypeInfoRef, TypeDescrBase*>
        TypeDescrBase::s_coalesced_type_table_map;

        std::vector<TypeDescrW>
        TypeDescrBase::s_type_table_v;

        TypeDescrW
        TypeDescrBase::require(std::type_info const * tinfo,
                               std::string_view canonical_name,
                               std::unique_ptr<TypeDescrExtra> tdextra)
        {
            /* 1. lookup by tinfo hash_code in s_type_table_map */
            {
                auto ix = s_type_table_map.find(TypeInfoRef(tinfo));

                if ((ix != s_type_table_map.end()) && ix->second)
                    return ix->second.get();
            }

            /* 2. lookup by tinfo hash_code in s_coalesced_type_table_map */
            {
                auto ix = s_coalesced_type_table_map.find(TypeInfoRef(tinfo));

                if ((ix != s_coalesced_type_table_map.end()) && ix->second)
                    return ix->second;
            }

            /* 3. O(n) lookup by canonical_name,  before we create a new slot.
             *
             * Have to accept that on clang type_info objects aren't always unique (!$@#!!)
             *
             * TODO: lookup table keyed by canonical_name
             */
            for (TypeDescrBase * x : s_type_table_v) {
                if (x && (x->canonical_name() == canonical_name)) {
                    /* 1. assume *x represents the type associated with tinfo.
                     * 2. *do* store tinfo in s_coalesced_type_table_map[],
                     *    for faster lookup next time
                     */
                    s_coalesced_type_table_map[TypeInfoRef(tinfo)] = x;

                    return x;
                }
            }

            TypeId id = TypeId::allocate();

            std::unique_ptr<TypeDescrBase> & slot = s_type_table_map[TypeInfoRef(tinfo)];

            slot.reset(new TypeDescrBase(id,
                                         tinfo,
                                         canonical_name,
                                         std::move(tdextra)));

            if (s_type_table_v.size() <= id.id())
                s_type_table_v.resize(id.id() + 1);

            s_type_table_v[id.id()] = slot.get();

            return slot.get();
        } /*require*/

        void
        TypeDescrBase::print_reflected_types(std::ostream & os)
        {
            os << "<type_table_v[" << s_type_table_v.size() << "]:";

            for (TypeDescrBase * td : s_type_table_v) {
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
                                     std::type_info const * native_tinfo,
                                     std::string_view canonical_name,
                                     std::unique_ptr<TypeDescrExtra> tdextra)
            : id_{std::move(id)},
              native_typeinfo_{native_tinfo},
              canonical_name_{std::move(canonical_name)},
              short_name_{unqualified_name(canonical_name_)},
              tdextra_{std::move(tdextra)}
        {
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
    } /*namespace reflect*/
} /*namespace xo*/

/* end TypeDescr.cpp */
