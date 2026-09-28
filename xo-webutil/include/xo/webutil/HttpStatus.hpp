/** @file HttpStatus.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

namespace xo {
    namespace web {
        /** @class HttpStatus
         *  @brief an http status code: strongly typed, converts to int.
         *
         *  Made from an int only explicitly, so a stray number cannot pass
         *  for a status.  Named ctors for the codes in use.
         **/
        class HttpStatus {
        public:
            explicit constexpr HttpStatus(int code) : code_{code} {}

            /** 200 **/
            static constexpr HttpStatus ok() { return HttpStatus(200); }
            /** 403 **/
            static constexpr HttpStatus forbidden() { return HttpStatus(403); }
            /** 404 **/
            static constexpr HttpStatus not_found() { return HttpStatus(404); }
            /** 500 **/
            static constexpr HttpStatus internal_error() { return HttpStatus(500); }

            constexpr int code() const { return code_; }
            constexpr operator int() const { return code_; }

            friend constexpr bool operator==(HttpStatus, HttpStatus) = default;

        private:
            /** e.g. 200 **/
            int code_;
        };
    } /*namespace web*/
} /*namespace xo*/

/* end HttpStatus.hpp */
