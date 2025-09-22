# xo-tokenizer/docs/CMakeLists.txt

xo_doxygen_collect_deps()
xo_docdir_doxygen_config()
xo_docdir_sphinx_config(
    index.rst install.rst examples.rst implementation.rst
    input-state-class.rst scan-result-class.rst schematika-tokens.rst span-class.rst
    token-class.rst tokenizer-error-class.rst tokentype-enum.rst
)
