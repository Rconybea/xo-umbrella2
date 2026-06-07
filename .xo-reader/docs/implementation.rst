.. _implementation:

.. toctree::
   :maxdepth: 2

Components
==========

Library dependency tower for *xo-reader*:

.. ditaa::

    +------------------------------------------+
    |                xo_reader                 |
    +-------------------------+----------------+
    |      xo_expression      |                |
    +-------------------------+                |
    |      xo_reflect         |  xo_tokenizer  |
    +-----------+-------------+                |
    |           |  xo_refcnt  |                |
    | xo_subsys +-------------+----------------+
    |           |          xo_indentlog        |
    +-----------+------------------------------+

Install instructions :doc:`here<install>`

Abstraction tower for *xo-reader* components:

.. ditaa::
    :--scale: 0.85

    +--------------------------------+
    |             reader             |
    +--------------------------------+
    |             parser             |
    +----------------+---------------+
    | exprstatestack | envframestack |
    +----------------+---------------+
    |   exprstate    |   envframe    |
    +----------------+---------------+

``exprstate`` provides an abstract api.
We use runtime polymorphism to represent concrete parsing states.
Different expression types inherit from ``exprstate`` to encapsulate
parsing for each expression type.

.. uml::
    :caption: exprstate
    :scale: 99%
    :align: center

    class exprstate {
    }

    class define_xs {
    }

    exprstate <|-- define_xs

    class lambda_xs {
    }

    exprstate <|-- lambda_xs

    class exprseq_xs {
    }

    exprstate <|-- exprseq_xs

    class let1_xs {
    }

    exprstate <|-- let1_xs

    class paren_xs {
    }

    exprstate <|-- paren_xs

    class sequence_xs {
    }

    exprstate <|-- sequence_xs

There are also classes for nested state machines:

.. uml::
    :caption: exprstate
    :scale: 99%
    :align: center

    class exprstate {
    }

    class progress_xs {
    }

    exprstate <|-- progress_xs

    class expect_symbol_xs {
    }

    exprstate <|-- expect_symbol_xs

    class expect_type_xs {
    }

    exprstate <|-- expect_type_xs

    class expect_expr_xs {
    }

    exprstate <|-- expect_expr_xs

    class expect_formal_xs {
    }

    exprstate <|-- expect_formal_xs

    class expect_formal_arglist_xs {
    }

    exprstate <|-- expect_formal_arglist_xs

Putting these in context:

.. list-table:: Schematika Parsing States
    :widths: 15 30
    :header-rows: 1

    * - exprstate class
      - target syntax
    * - define_xs
      - ``def foo : f64 = 1;``, ``def sq = lambda (x : i64) { x * x; }``
    * - progress_xs
      - possibly-incomplete arithmetic expressions
        ``(a + b) * 7``..
