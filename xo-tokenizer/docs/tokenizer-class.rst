
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
    +-----------------+-----------------------+          |
    |                 |    tokenizer_error    |  buffer  |
    |     token       +-----------------------+          |
    |                 |      input_state      |          |
    +-----------------+-----------------------+          |
    |    tokentype    |          span         |          |
    +-----------------+-----------------------+----------+

.. code-block:: cpp

    #include <xo/tokenizer/tokenizer.hpp>

.. uml::
    :scale: 99%
    :align: center

    allowmixing

    object tkz1<<tokenizer>>
    tkz : input_state

    object ins1<<input_state>>

    tkz1 o-- ins1

- Assemble a stream of lexical tokens from a text stream.

- Lexical errors reported via scan_result instance;
  errors reported with detailed context

Class
-----

.. doxygenclass:: xo::scm::tokenizer

Instance Variables
------------------

.. doxygenclass:: tokenizer-instance-vars

Constructors
------------

.. doxygengroup:: tokenizer-ctors
