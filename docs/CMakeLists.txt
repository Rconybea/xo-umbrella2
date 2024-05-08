# xo-unit/docs/CMakeLists.txt

xo_doxygen_collect_deps()
xo_docdir_doxygen_config()
xo_docdir_sphinx_config(
    index.rst examples.rst glossary.rst install.rst implementation.rst
    quantity-reference.rst quantity-class.rst quantity-factoryfunctions.rst quantity-unitvars.rst
    unit-reference.rst unit-concept.rst unit-quantities.rst basis-unit-class.rst dimension-enum.rst
)
#xo_utest_coverage_config2()
