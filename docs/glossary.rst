.. _glossary:

Glossary
--------

.. glossary::
    basis dimension
      Orthogonal directions associated with basis units,  for example *mass*, *length*, *time*.
      In xo-unit these are represented by the enum ``xo::qty::dim``.

    basis unit (bu)
      An implementation type representing a quantity (with associated scale) in the direction of a single :term:`basis dimension`.
      For example *milliseconds*, *seconds*, and *hours* stand for different basis units with the *time* dimension.
      In xo-unit these are represented by the template type ``xo::qty::basis_unit``.

    bpu
      A rational power of a (single) basis unit.  For example *s\ :sup:-2* or *hr\ :sup:-(1/2)*.
      In xo-unit these are represented by the template type ``xo::qty::bpu``.

    natural unit (nu)
      A cartesian product of basis power units (bpus).  For example *kg.m.s\ :sup:-2* or *hr\ :sup:-(1/2)*.
      In xo-unit these are represented by template type ``xo::qty::natural_unit``.

    scaled unit (su)
      A dimensionless multiple of a natural unit.  Used as intermediate value when coalescing quantities involving different units.
      In xo-unit these are represented by template type ``xo::qty::scaled_unit``.

    XO
      A set of integrated c++ libraries for complex event processing, with browser and python integration.
      `xo documentation here`_

.. _xo documentation here: https://rconybea.github.io/web/sw/xo.html

.. toctree::
