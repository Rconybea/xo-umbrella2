
.. _tokenizer-class:

Tokenizer
=========

Parse a Schematika character stream into lexical tokens

Context
-------

.. ditaa::
    :--scale: 0.85

    +-----------------------------------------+----------+
    |cBLU            tokenizer                |          |
    +-----------------------------------------+          |
    |               scan_result               |          |
    +-----------------+-----------------------+  buffer  |
    |     token       |    tokenizer_error    |          |
    +-----------------+-----------------------+          |
    |    tokentype    |          span         |          |
    +-----------------+-----------------------+----------+

.. code-block:: cpp

    #include <xo/tokenizer/tokenizer.hpp>
