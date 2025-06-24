
.. _tokenizer-error-class

Tokenizer Error
===============

Represent a possible tokenizer error result, including parsing context

Context
-------

.. ditaa::
    :--scale: 0.85

    +-----------------------------------------+----------+
    |                tokenizer                |          |
    +-----------------------------------------+          |
    |               scan_result               |          |
    +-----------------+-----------------------+  buffer  |
    |     token       |cBLU tokenizer_error   |          |
    +-----------------+-----------------------+          |
    |    tokentype    |          span         |          |
    +-----------------+-----------------------+----------+

.. code-block:: cpp

    #include <xo/tokenizer/tokenizer_error.hpp>

Class
------

.. doxygenclass:: xo::scm::tokenizer_error

Instance Variables
------------------

.. doxygengroup:: tokenizer-error-instance-vars

Constructors
------------

.. doxygengroup:: tokenizer-error-ctors

Access Methods
--------------

.. doxygengroup:: tokenizer-error-access-methods

General Methods
---------------

.. doxygengroup:: tokenizer-error-general-methods
