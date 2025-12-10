/** @file objectmodel.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "xo/facet/AFacet.hpp"
#include <catch2/catch.hpp>
#include <cmath>
#include <cassert>
#include <cstring>

namespace xo {
    using xo::facet::valid_abstract_facet;

    namespace ut {
        // ------ AComplex -----

        /** abstract interface for a complex number **/
        struct AComplex {
            using TypeErasedIface = struct IComplex_Any;

            virtual double xcoord(void * data) const = 0;
            virtual double ycoord(void * data) const = 0;
            virtual double argument(void * data) const = 0;
            virtual double magnitude(void * data) const = 0;

            virtual void destruct_data(void * data) const = 0;

        private:
            static bool _valid;
        };

        bool
        AComplex::_valid = valid_abstract_facet<AComplex>();

        // ----- IComplex_Any -----

        /** type-erased implementation of AComplex, for runtime polymorphism
         *  Usable by (and only by) overwriting with a typed implementation,
         *  such as IComplex_RectCoords or IComplex_PolarCoords.
         **/
        struct IComplex_Any : public AComplex {
            virtual double xcoord(void *) const final override { assert(false); return 0.0; }
            virtual double ycoord(void *) const final override { assert(false); return 0.0; }
            virtual double argument(void *) const final override { assert(false); return 0.0; }
            virtual double magnitude(void *) const final override { assert(false); return 0.0; }

            virtual void destruct_data(void *) const final override { assert(false); }

        private:
            static bool _valid;
        };

        bool
        IComplex_Any::_valid = true; //valid_interface_implementation<AComplex, IComplex_Any>();


    }
}

/* end objectmodel.test.cpp */
