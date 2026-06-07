/** @file SchematikaError.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include <string>

namespace xo {
    namespace scm {
        class SchematikaError {
        public:
            SchematikaError() = default;
            explicit SchematikaError(std::string x) : what_{std::move(x)} {}

            const std::string & what() const { return what_; }

            bool is_error() const { return !what_.empty(); }
            bool is_not_an_error() const { return what_.empty(); }

        private:
            std::string what_;
        };
    }
}

/* end SchematikaError.hpp */
