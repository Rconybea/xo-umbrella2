.. xo-tokenizer documentation master file.

xo-tokenizer documentation
==========================

xo-tokenizer provides a tokenizer for the Schematika language.

Syntax is generally C-like, but with some important differences.
Notably, characters used for arithmetic operators (``+``, ``-``, ``*``, ``/``)
may appear in variable names:  ``one-of-those-days`` is an ordinary symbol.

Typically applications would use xo-reader to construct Schematika expressions
instead of interacting directly with ``xo::scm::tokenizer``.

.. toctree::
   :maxdepth: 2
   :caption: xo-tokenizer contents

   install
   examples
   schematika-tokens
   implementation
   tokenizer-class
   scan-result-class
   token-class
   tokenizer-error-class
   input-state-class
   span-class
   tokentype-enum
