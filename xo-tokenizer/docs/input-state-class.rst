
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
