.. _glossary:

Glossary
--------

.. glossary::
    dimension
    dim
      | Fundamental, orthogonal directions associated available for constructing units.
      | For example *mass*, *length*, *time*.
      | In *xo-unit* these are represented by the enum :doc:`xo::qty::dimension<dimension-enum>`.

    basis unit
    bu
      | An implementation type representing a quantity (with associated scale) in the direction of a single :term:`dimension`.
      | For example *milliseconds*, *seconds*, and *hours* stand for different basis units within the *time* dimension.
      | In *xo-unit* these are represented by the template type :doc:`xo::qty::basis_unit<basis-unit-class>`.

    basis power unit
    bpu
      | A rational power of a (single) basis unit.
      | For example :math:`s^{-2}` for unit "per second squared" or :math:`yr^{-(1/2)}` for "per square-root of a year".
      | In *xo-unit* these are represented by the template type :doc:`xo::qty::bpu<bpu-class>`

    natural unit
    nu
      | A cartesian product of basis power units (bpus);
      | For example :math:`kg.m.s^{-2}` or :math:`hr^{-(1/2)}`.
      | In *xo-unit* these are represented by template type :doc:`xo::qty::natural_unit<natural-unit-class>`.

    scaled unit
    su
      | A dimensionless multiple of a natural unit.
      | Used as intermediate value when coalescing quantities involving different units.
      | In *xo-unit* these are represented by template type :doc:`xo::qty::scaled_unit<scaled-unit-class>`.

    XO
      A set of integrated c++ libraries for complex event processing, with browser and python integration.
      `xo documentation here`_

.. _xo documentation here: https://rconybea.github.io/web/sw/xo.html

.. toctree::
