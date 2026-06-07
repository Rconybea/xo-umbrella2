
.. _token-class:

Token
=====

Represent a single lexical token in the Schematika language

Context
-------

.. ditaa::
    :--scale: 0.85

    +-----------------------------------------+----------+
    |                tokenizer                |          |
    +-----------------------------------------+          |
    |               scan_result               |          |
    +-----------------+-----------------------+          |
    |cBLU             |     tokenizer_error   |  buffer  |
    |     token       +-----------------------+          |
    |                 |      input_state      |          |
    +-----------------+-----------------------+          |
    |     tokentype   |          span         |          |
    +-----------------+-----------------------+----------+

.. code-block:: cpp

    #include <xo/tokenizer/token.hpp>

.. uml::
    :scale: 99%
    :align: center

    allowmixing

    object tk1<<token>>
    tk1 : tk_type = tk_i64
    tk1 : text = "123"

    object tk2<<token>>
    tk2 : tk_type = tk_string
    tk2 : text = "the quick brown fox"

- Represent a single lexical token

- Does not share any storage with original input stream
  (maintains a local copy).

- Remembers copied input extent.
  Convert on demand to native untagged representation

Example
-------

.. code-block:: cpp

    void foo() {
        using namespace xo::scm;

        token<char> tk = token<char>::i64_token("123");

        tk.is_valid(); // -> true
        tk.text(); // -> "123"s;

        tk.tk_type(); // -> tokentype::tk_i64
        tk.i64_value(); // -> 123

        cout << tk << endl; // -> <token :type i64 :text 123>
    }

Class
-----

.. doxygenclass:: xo::scm::token


Instance Variables
------------------

.. doxygengroup:: token-instance-vars

Constructors
------------

.. doxygengroup:: token-ctors

Access Methods
--------------

.. doxygengroup:: token-access-methods

General Methods
---------------

.. doxygengroup:: token-general-methods
