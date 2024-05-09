.. _development:

Miscellaneous development notes for *xo-unit*.

How To...
=========

Add Basis Unit
--------------

To add a basis unit for an existing dimension:

#. add unit definition to the ``xo::qty::bu`` namespace in ``include/xo/unit/basis_unit.hpp``
#. add call to ``bu_store::bu_establish_abbrev()`` from  ``bu_store::bu_store``.
#. add ``natural_unit`` definition to ``xo::qty::nu`` namespace in ``include/xo/unit/natural_unit.hpp``
#. add ``scaled_unit`` definition to ``xo::qty::u`` namespace in ``include/xo/unit/scaled_unit.hpp``.
#. add unit quantity to ``xo::qty::qty`` namespace in ``include/xo/unit/quantity.hpp``
#. add factory function to ``xo::qty::qty`` namespace in ``include/xo/unit/quantity.hpp``
