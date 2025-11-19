/* file GlobalSymtab.hpp
 *
 * author: Roland Conybeare, Jun 2024
 */

#pragma once

#include "SymbolTable.hpp"
#include <map>
#include <string>

namespace xo {
    namespace scm {
        class GlobalSymtab : public SymbolTable {
        public:
            /** create instance.  Probably only need one of these **/
            static rp<GlobalSymtab> make_empty() { return new GlobalSymtab(); }

            bp<Expression> require_global(const std::string & vname,
                                          bp<Expression> expr);

            // ----- Environment -----

            virtual bool is_global_env() const override { return true; }

            virtual binding_path lookup_binding(const std::string & /*vname*/) const override {
                /* i_link: -1 for global environment
                 * j_slot: not used
                 */
                return { -1, 0 };
            }

            virtual bp<Expression> lookup_var(const std::string & vname) const override {
                return this->lookup_local(vname);
            }

            virtual bp<Expression> lookup_local(const std::string & vname) const override {
                auto ix = global_map_.find(vname);

                if (ix == global_map_.end()) {
                    /* not found */
                    return bp<Variable>::from_native(nullptr);
                }

                return ix->second;
            }

            virtual void upsert_local(bp<Variable> target) override;

            virtual void print(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const xo::print::ppindentinfo & ppii) const override;

        private:
            GlobalSymtab();

        private:
            /* for assignable globals,  need to allocate memory
             * addresses for these.
             */
            std::map<std::string, rp<Expression>> global_map_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/* end GlobalSymtab.hpp */
