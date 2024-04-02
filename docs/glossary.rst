.. _glossary:

Glossary
--------

.. glossary::
    basis dimension
      Orthogonal directions associated with basis units,  for example *mass*, *length*, *time*.
      In xo-unit these are represented by the enum ``xo::unit::dim``.

    basis unit
      An implementation type representing a quantity (with associated scale) in the direction of a single :term:`basis dimension`.
      For example *milliseconds*, *seconds*, and *hours* stand for different basis units with the *time* dimension.
      In xo-unit these are represented by the template type ``xo::unit::basis_unit``.

    bpu
      A rational power of a (single) basis unit.  For example *kg.m.s\ :sup:-2* or *hr\ :sup:-(1/2)*.
      In xo-unit these are represented by the template type ``xo::unit::bpu``.

    XO
      A set of integrated c++ libraries for complex event processing, with browser and python integration.
      `xo documentation here`_

.. _xo documentation here: https://rconybea.github.io/web/sw/xo.html

.. toctree::
