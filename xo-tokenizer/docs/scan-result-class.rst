
.. _scan-result-class:

Scan Result
===========

Represent the result of a tokenizer scan call

Context
-------

.. ditaa::
    :--scale: 0.85

    +-----------------------------------------+----------+
    |                tokenizer                |          |
    +-----------------------------------------+          |
    |cBLU           scan_result               |          |
    +-----------------+-----------------------+          |
    |                 |    tokenizer_error    |  buffer  |
    |     token       +-----------------------+          |
    |                 |      input_state      |          |
    +-----------------+-----------------------+          |
    |    tokentype    |          span         |          |
    +-----------------+-----------------------+----------+

.. code-block:: cpp

    #include <xo/tokenizer/input_state.hpp>
