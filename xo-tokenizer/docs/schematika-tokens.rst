.. _schematika-tokens:

Schematika Tokens
=================

.. list-table:: Schematika Tokens
   :widths: 15 30 30
   :header-rows: 1

   * - tokentype
     - examples
     - description
   * - tk_i64
     - ``123``, ``-8``
     - 64-bit integer literal
   * - tk_f64
     - ``1.234``, ``-10``., ``-1.981e-10``, ``3e6``
     - 64-bit floating-point literal
   * - tk_string
     - ``"hello"``, ``"Q: \"what's up?\"\nA: \"parsing!\""``
     - string literal. Usual escapes ``\n``, ``\r``, ``\t``, ``\"``, ``\\``
   * - tk_symbol
     - ``apple``, ``funKy``, ``x123``, ``_mumble``, ``hyphenated-var``
     - symbol name
   * - tk_type
     - ``type``
     - keyword
   * - tk_def
     - ``def``
     - keyword
   * - tk_lambda
     - ``lambda``
     - keyword
   * - tk_if
     - ``if``
     - keyword
   * - tk_let
     - ``let``
     - keyword
   * - tk_in
     - ``in``
     - keyword
   * - tk_end
     - ``end``
     - keyword
   * - tk_leftparen
     - ``(``
     -
   * - tk_rightparen
     - ``)``
     -
   * - tl_leftbracket
     - ``[``
     -
   * - tk_rightbracket
     - ``]``
     -
   * - tk_leftbrace
     - ``{``
     -
   * - tk_rightbrace
     - ``}``
     -
   * - tk_leftangle
     - ``<``
     -
   * - tk_rightangle
     - ``>``
     -
   * - tk_dot
     - ``.``
     -
   * - tk_comma
     - ``,``
     -
   * - tk_colon
     - ``:``
     -
   * - tk_doublecolon
     - ``::``
     -
   * - tk_semicolon
     - ``;``
     -
   * - tk_singleassign
     - ``=``
     -
   * - tk_assign
     - ``:=``
     -
   * - tk_yields
     - ``->``
     -
   * - tk_plus
     - ``+``
     - allowed in symbol
   * - tk_minus
     - ``-``
     - allowed in symbol
   * - tk_star
     - ``*``
     - allowed in symbol
   * - tk_slash
     - ``/``
     - allowed in symbol
