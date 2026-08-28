/* @file print_eigen.hpp */

#include <xo/ppsink/Prettifier.hpp>
#include <Eigen/Dense>
#include <cstdint>

namespace xo::pp {
    template<typename T>
    class matrix {
    public:
        matrix(T & x) : x_{x} {}

        /* print this value */
        const T & x_;
    }; /*matrix*/

#ifdef OBSOLETE
    template<typename T>
    inline std::ostream &
    operator<<(std::ostream & s, matrix<T> const & mat)
    {
        s << "[";
        for(std::uint32_t i = 0, m = mat.x_.rows(); i<m; ++i) {
            if(i > 0)
                s << "; ";

            for(std::uint32_t j = 0, n = mat.x_.cols(); j<n; ++j) {
                if(j > 0)
                    s << ' ';

                s << mat.x_(i, j);
            }
        }
        s << "]";

        return s;
    } /*operator<<*/
#endif

    template <typename T>
    inline void pretty_matrix(PpSink & sink, const T & x)
    {
        sink.put("[");
        /* begin_here, not begin: take the indent origin from the CURRENT
         * output column, so a matrix that breaks aligns its rows under the
         * opening bracket rather than at the enclosing record's indent.
         * begin_here falls back to begin on sinks that do not override it
         * (FlatSink), so flat rendering is unaffected.
         */
        sink.begin_here(0);

        for (auto i = 0l, m = x.rows(); i < m; ++i) {
            if (i > 0) {
                sink.put(";");
                sink.split(1, 0);
            }

            for (auto j = 0l, n = x.cols(); j < n; ++j) {
                if (j > 0)
                    sink.put(" ");

                sink.pp(x(i,j));
            }
        }

        sink.put("]");
        sink.end();
    }

    template <typename T>
    class Prettifier<matrix<T>> {
    public:
        static void print(PpSink & sink, const matrix<T> & x) {
            pretty_matrix(sink, x.x_);
        }
    };

} /*namespace xo::pp*/

/* end print_eigen.hpp */
