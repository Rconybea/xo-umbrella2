
.. _input-state-class:

Input State
===========

Track detailed state of input stream to collect information useful for detailed error reporting

Context
-------

.. ditaa::
    :--scale: 0.85

    +-----------------------------------------+----------+
    |                tokenizer                |          |
    +-----------------------------------------+          |
    |               scan_result               |          |
    +-----------------+-----------------------+          |
    |                 |    tokenizer_error    |  buffer  |
    |     token       +-----------------------+          |
    |                 |cBLU  input_state      |          |
    +-----------------+-----------------------+          |
    |    tokentype    |          span         |          |
    +-----------------+-----------------------+----------+

.. code-block:: cpp

    #include <xo/tokenizer/input_state.hpp>

.. uml::
    :scale: 99%
    :align: center

    allowmixing

    object in1<<input_state>>
    in1 : current_line = input
    in1 : current_pos
    in1 : whitespace
    in1 : debug_flag

    object input
    input : (x * y * 123d)

    input o-- sp1


Class
-----

.. doxygenclass:: xo::scm::input_state

Instance Variables
------------------

.. doxygengroup:: input-state-instance-vars

Constructors
------------

.. doxygengroup:: input-state-ctors

Static Methods
--------------

.. doxygengroup:: input-state-static-methods

Access Methods
--------------

.. doxygengroup:: input-state-access-methods

General Methods
---------------

.. doxygengroup:: input-state-general-methods
