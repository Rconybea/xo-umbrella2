/** @file LineState.cpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#include "LineState.hpp"
#include <iostream>
#include <cassert>

namespace xo {
    using std::cerr;
    using std::endl;
    using std::size_t;

    LineState::LineState(size_t local_ppos,
                         size_t solpos,
                         size_t color_escape_chars,
                         ptrdiff_t color_escape_start)
        : local_ppos_{local_ppos},
          solpos_{solpos},
          color_escape_chars_{color_escape_chars},
          color_escape_start_{color_escape_start}
    {}

    void
    LineState::_check_update_local_state(const char * p0,
                                         const char * pn,
                                         bool debug_flag)
    {
        if (debug_flag) {
            cerr << "_check_update_local_state:" << endl;
            cerr << "  buf: (p0=" << (void*)p0 << ", pn=" << (void*)pn << ")" << std::endl;
            cerr << "  solpos_=" << solpos_
                 << ", color_escape_chars_=" << color_escape_chars_
                 << endl;
        }

        if (p0 + local_ppos_ == pn) [[likely]] {
            // solpos_, color_escape_chars_, color_escape_start_ all up-to-date
        } else {
            // [pnew, pn): input arriving at this accountant for the first time,
            // no already incorporated into
            //   {solpos_, color_escape_chars_, color_escape_start_)

            const char * pnew = p0 + this->local_ppos_;

            if (debug_flag) {
                cerr << "_check_update_local_state:" << endl;
                cerr << "  range: (pnew=" << (void*)pnew << ", pn=" << (void*)pn << ")"
                     << endl;
            }

            for(const char * p = pnew; p < pn; ++p) {
                this->_update_local_state_char(p0, p, debug_flag);
            }

            // solpos_, color_escape_chars_, color_escape_start_ all up-to-date
            // for current buffered contents

            this->local_ppos_ = pn - p0;

            if (debug_flag) {
                cerr << "_check_update_local_state:" << endl;
                cerr << "  pos=" << local_ppos_;
                cerr << ", solpos=" << solpos_;
                cerr << ", color_escape_chars=" << color_escape_chars_
                     << endl;
            }

            assert(local_ppos_ >= solpos_ + color_escape_chars_);
        }
    }

    void
    LineState::_update_local_state_char(const char * p0,
                                        const char * p,
                                        bool debug_flag)
    {
        if ((*p == '\n') || (*p == '\r')) {
            this->solpos_ = (p+1 - p0);
            /* reset, since these chars only relevant as correction to solpos */
            this->color_escape_chars_ = 0;
            /* -> incomplete color escape, broken by newline */
            this->color_escape_start_ = -1;
        } else if (*p == '\033') {
            if (debug_flag) [[unlikely]] {
                std::cout << "xsputn: \\033 at p-p0=" << (p - p0) << std::endl;
            }
            this->color_escape_start_ = p - p0;
        } else if (this->color_escape_start_ >= 0) {
            if (*p == 'm') {
                /* escape sequence is non-printing including both endpoints */
                std::int64_t esc_chars = (p+1 - (p0 + color_escape_start_));

                this->color_escape_chars_ += esc_chars;

                if (debug_flag) [[unlikely]] {
                    cerr << "xsputn: m at p-p0" << (p - p0) << " +" << esc_chars
                         << " -> color_escape_chars=" << color_escape_chars_
                         << endl;
                }
                this->color_escape_start_ = -1;
            } else if (!isdigit(*p) && (*p != '[') && (*p != ';')) {
                /* not color escape after all */
                this->color_escape_start_ = -1;
            }
        }
    }



} /*namespace xo*/

/* end LineState.cpp */
