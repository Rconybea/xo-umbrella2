/** @file DDictionary.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "DDictionary.hpp"
#include "Array.hpp"
#include "String.hpp"
#include <xo/facet/FacetRegistry.hpp>
#include <utility>

namespace xo {
    using xo::print::APrintable;
    using xo::facet::FacetRegistry;
    using xo::mm::AGCObject;

    namespace scm {

        DDictionary::DDictionary(DArray * keys,
                                 DArray * values)
            : keys_{keys}, values_{values}
        {}

        DDictionary *
        DDictionary::empty(obj<AAllocator> mm, size_type cap)
        {
            void * mem = mm.alloc_for<DDictionary>();

            if (mem) {
                if (cap <= 0)
                    cap = 1;

                DArray *   keys = DArray::_empty(mm, cap);
                DArray * values = DArray::_empty(mm, cap);

                if (keys && values)
                    return new (mem) DDictionary(keys, values);
            }

            return nullptr;
        }

        std::optional<obj<AGCObject>>
        DDictionary::lookup(const DString * key) const noexcept
        {
            for (DArray::size_type i = 0, z = keys_->size(); i < z; ++i) {
                auto i_key = obj<AGCObject,DString>::from(keys_->at(i));

                assert(i_key);

                if (DString::compare(*key, *i_key.data()) == 0)
                    return values_->at(i);
            }

            return {};
        }

        std::optional<obj<AGCObject>>
        DDictionary::lookup_cstr(const char * key) const noexcept
        {
            for (DArray::size_type i = 0, z = keys_->size(); i < z; ++i) {
                auto i_key = obj<AGCObject,DString>::from(keys_->at(i));

                assert(i_key);

                if (strcmp(key, i_key->data()) == 0)
                    return values_->at(i);
            }

            return {};
        }

        std::pair<const DString *, obj<AGCObject>>
        DDictionary::at_index(size_type ix) const
        {
            if (ix < keys_->size()) {
#ifndef NDEBUG
                auto key_str = obj<AGCObject,DString>::from((*keys_)[ix]);
                assert(key_str);
#endif

                return pair_type(this->key_at_index(ix), (*values_)[ix]);
            }

            return pair_type();
        }

        const DString *
        DDictionary::key_at_index(size_type ix) const
        {
            auto key_str = obj<AGCObject,DString>::from((*keys_)[ix]);

            assert(key_str);

            return key_str.data();
        }

        obj<AGCObject>
        DDictionary::value_at_index(size_type ix) const
        {
            if (ix < keys_->size()) {
                assert(ix < values_->size());

                return (*values_)[ix];
            }

            return obj<AGCObject>();
        }

        bool
        DDictionary::try_update(obj<AAllocator> mm, const pair_type & kv_pair)
        {
            for (size_type i = 0, n = keys_->size(); i < n; ++i) {
                auto key_i = obj<AGCObject,DString>::from((*keys_)[i]);

                assert(key_i);

                if (*(key_i.data()) == *(kv_pair.first)) {
                    values_->assign_at(mm, i, kv_pair.second);
                    return true;
                }
            }

            return false;
        }

        bool
        DDictionary::try_update_cstr(obj<AAllocator> mm, const char * key, obj<AGCObject> value)
        {
            for (size_type i = 0, n = keys_->size(); i < n; ++i) {
                auto key_i = obj<AGCObject,DString>::from((*keys_)[i]);

                assert(key_i);

                if (strcmp(key, key_i->data()) == 0) {
                    values_->assign_at(mm, i, value);

                    return true;
                }
            }

            return false;
        }

        bool
        DDictionary::try_upsert_cstr(obj<AAllocator> mm, const char * key_cstr, obj<AGCObject> value)
        {
            const DString * k1 = DString::from_cstr(mm, key_cstr);

            if (k1) {
                return this->try_upsert(mm, std::make_pair(k1, value));
            }

            return false;
        }

        bool
        DDictionary::upsert_cstr(obj<AAllocator> mm, const char * key_cstr, obj<AGCObject> value)
        {
            const DString * k1 = DString::from_cstr(mm, key_cstr);

            if (k1)
                return this->upsert(mm, std::make_pair(k1, value));

            return false;
        }

        bool
        DDictionary::try_upsert(obj<AAllocator> mm, const pair_type & kv_pair)
        {
            if (this->try_update(mm, kv_pair))
                return true;

            if (keys_->size() == keys_->capacity())
                return false;

            return this->_append_kv_aux(mm, kv_pair);
        }

        bool
        DDictionary::upsert(obj<AAllocator> mm, const pair_type & kv_pair)
        {
            if (this->try_update(mm, kv_pair))
                return true;

            // key not present -> must expand {key array, value array}

            if (keys_->size() == keys_->capacity()) {
                assert(keys_->capacity() > 0);

                size_type cap_2x = 2 * keys_->capacity();

                DArray * keys_2x = DArray::copy(mm, keys_, cap_2x);
                DArray * values_2x = DArray::copy(mm, values_, cap_2x);

                if (keys_2x && values_2x) {
                    this->keys_ = keys_2x;
                    this->values_ = values_2x;
                } else {
                    return false;
                }
            }

            return this->_append_kv_aux(mm, kv_pair);
        }

        bool
        DDictionary::_append_kv_aux(obj<AAllocator> mm, const pair_type & kv_pair)
        {
            DString * key = const_cast<DString *>(kv_pair.first);

            bool ok
                = keys_->push_back(mm, obj<AGCObject,DString>(key));

            if (ok) {
                ok = values_->push_back(mm, kv_pair.second);

                if (!ok) {
                    // since we couldn't insert value, also drop key

                    keys_->pop_back();
                }
            }

            return ok;
        }

        void
        DDictionary::shrink_to_fit() noexcept
        {
            keys_->shrink_to_fit();
            values_->shrink_to_fit();
        }

        // ----- printable facet ----

        bool
        DDictionary::pretty(const ppindentinfo & ppii) const
        {
            using xo::print::ppstate;

            ppstate * pps = ppii.pps();

            if (ppii.upto()) {
                pps->write("{");

                for (size_type i = 0, n = this->size(); i < n; ++i) {
                    pps->write(" ");

                    obj<APrintable> key
                        = FacetRegistry::instance().variant<APrintable,AGCObject>((*keys_)[i]);
                    obj<APrintable> value
                        = FacetRegistry::instance().variant<APrintable,AGCObject>((*values_)[i]);

                    assert(key.data());
                    assert(value.data());

                    if (!pps->print_upto(key))
                        return false;
                    pps->write(": ");

                    if (!pps->print_upto(value))
                        return false;
                    pps->write(";");
                }

                pps->write(" }");
                return true;
            } else {
                pps->write("{");

                for (size_type i = 0, n = this->size(); i < n; ++i) {
                    if (i == 0) {
                        /* indent, but credit initial {. using same line for first (key,value) */
                        pps->indent(std::max(pps->indent_width(), 1u) - 1);
                    } else {
                        /* indent after newline */
                        pps->newline_indent(ppii.ci1());
                    }

                    obj<APrintable> key
                        = FacetRegistry::instance().variant<APrintable,AGCObject>((*keys_)[i]);
                    obj<APrintable> value
                        = FacetRegistry::instance().variant<APrintable,AGCObject>((*values_)[i]);

                    pps->pretty(key);
                    pps->write(": ");
                    pps->pretty(value);
                    pps->write(";");
                }

                pps->write(" }");
                return false;
            }
        }

        // ----- gcobject facet -----

        DDictionary *
        DDictionary::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DDictionary::visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &keys_);
            gc.visit_child(reason, &values_);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DDictionary.cpp */
