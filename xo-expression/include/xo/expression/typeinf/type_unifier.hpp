/** @file type_unifier.hpp **/

#include "type_ref.hpp"
#include "TypeBlueprint.hpp"
#include <map>

namespace xo {
    namespace scm {
        struct unify_result {
            /** true iff unification success **/
            bool success_;
            /** blueprint (possibly concrete) for unified type **/
            rp<TypeBlueprint> unified_;
            /** if @ref success_ is false -> non-null source function
             *  in which contradiction detected
             **/
            const char * error_src_function_ = nullptr;
            /** if @ref success_ is false -> human-readable error description **/
            std::string error_description_;
        };

        std::ostream & operator<< (std::ostream & os, const unify_result & x);

        /** @class type_unifer
         *  @brief type unification algorithm
         **/
        class type_unifier {
        public:
            type_unifier() = default;

            /** error message where unification would require both
             *  1. equals(s1,t2)
             *  2. t2 contains s1
             **/
            static unify_result occurs_error(const char * src_function,
                                             bp<TypeBlueprint> t1,
                                             bp<TypeBlueprint> t2,
                                             bp<TypeBlueprint> s1,
                                             bp<TypeBlueprint> s2);

            /** given fact that @p lhs and @p rhs must refer to
             *  the same type:
             *  1. unify their type blueprints to get new blueprint U(lhs.rhs)
             *  2. update @ref constraint_map_ so that typevar ids for
             *     @p lhs and @p rhs refer to U(lhs,rhs)
             *  3. also update @ref constraint_map_ for any secondary unifications
             *     that are discovered
             *  4. return error if unification is contradiction encountered.
             **/
            unify_result unify(bp<TypeBlueprint> lhs, bp<TypeBlueprint> rhs);

        private:
            type_substitution_map constraint_map_;
        };

    } /*namespace scm*/
} /*namespace xo*/


/** end type_unifier.hpp **/
