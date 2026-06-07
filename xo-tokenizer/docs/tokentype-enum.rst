
.. _tokentype-enum:

Tokentype
=========

Distinguish different lexical tokens for the Schematika language.

Context
-------

.. ditaa::
    :--scale: 0.85

    +-----------------------------------------+----------+
    |                tokenizer                |          |
    +-----------------------------------------+          |
    |               scan_result               |          |
    +-----------------+-----------------------+          |
    |                 |     tokenizer_error   |  buffer  |
    |     token       +-----------------------+          |
    |                 |      input_state      |          |
    +-----------------+-----------------------+          |
    |cBLU tokentype   |          span         |          |
    +-----------------+-----------------------+----------+

.. code-block:: cpp

    #include <xo/tokenizer/tokentype.hpp>

Enum
----

.. doxygenfunction:: xo::scm::tokentype_descr

.. doxygenfunction:: xo::scm::operator<<(std::ostream&,tokentype)
