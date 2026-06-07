.. _implementation:

.. toctree::
   :maxdepth: 2

Components
==========

Library dependency tower for *xo-tokenizer*:

.. ditaa::

    +-----------------+
    |  xo_tokenizer   |
    +-----------------+
    |  xo_indentlog   |
    +-----------------+
    |    xo_cmake     |
    +-----------------+

Install instructions :doc:`here<install>`

Abstraction tower for *xo-tokenizer* components:

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
    |     tokentype   |          span         |          |
    +-----------------+-----------------------+----------+
